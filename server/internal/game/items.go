package game

import (
	"sort"
	"time"

	"tap/server/internal/logger"
	"tap/server/internal/protocol"
)

// Items are unique instances: each one has a serial number (per item ID, from 0) and is in exactly
// one place: on a room floor, in one inventory, or gone. The serial stays on the server (logs);
// the protocol only shows the item ID, so "egg" can appear twice in a list.
//
// World rules:
//   - A player carries at most one item of each ID.
//   - An item ID never exists more times than there are players online.
//   - A spawn point (item placed in a room of world.json) holds one item. After it is taken,
//     it gives a new one once ItemRespawnDelay has passed, if the item exists less than players online.
//   - An item dropped by a player disappears after DropLifetime and never counts as a spawn point item.

const (
	ItemRespawnDelay = 30 * time.Second
	DropLifetime     = 30 * time.Second
)

type Item struct {
	ID     string
	Serial int
}

// spawnPoint is one item placed in one room by world.json.
type spawnPoint struct {
	Room       string
	ItemID     string
	Present    bool // its item is on the floor
	EmptySince time.Time
}

type floorItem struct {
	Item    *Item
	Expires time.Time   // dropped items only
	Point   *spawnPoint // spawn point items only
}

func (s *State) mint(id string, reason string) *Item {
	item := &Item{ID: id, Serial: s.nextSerial[id]}
	s.nextSerial[id]++

	logger.Info("item_created", logger.Fields{"item": id, "serial": item.Serial, "reason": reason})

	return item
}

// deleteItem is the end of an instance (used, delivered, expired, owner disconnected).
func (s *State) deleteItem(item *Item, reason string) {
	logger.Info("item_deleted", logger.Fields{"item": item.ID, "serial": item.Serial, "reason": reason})
}

// total counts every instance of an item: floors and inventories.
func (s *State) total(id string) int {
	count := 0

	for _, entries := range s.floor {
		for _, entry := range entries {
			if entry.Item.ID == id {
				count++
			}
		}
	}

	for _, player := range s.players {
		for _, item := range player.Inventory {
			if item.ID == id {
				count++
			}
		}
	}

	return count
}

func holds(player *Player, id string) bool {
	for _, item := range player.Inventory {
		if item.ID == id {
			return true
		}
	}

	return false
}

func (s *State) floorIDs(roomID string) []string {
	ids := make([]string, 0)

	for _, entry := range s.floor[roomID] {
		ids = append(ids, entry.Item.ID)
	}

	return ids
}

// addFloor keeps a floor ordered by item ID then serial: LOOK lists in that order and TAKE gives the lowest serial.
func (s *State) addFloor(roomID string, entry *floorItem) {
	entries := append(s.floor[roomID], entry)

	sort.Slice(entries, func(i, j int) bool {
		a, b := entries[i].Item, entries[j].Item
		if a.ID != b.ID {
			return a.ID < b.ID
		}

		return a.Serial < b.Serial
	})

	s.floor[roomID] = entries
}

// removeFloor takes an entry off a floor; a spawn point starts its countdown.
func (s *State) removeFloor(roomID string, entry *floorItem) {
	entries := s.floor[roomID]

	for index, current := range entries {
		if current == entry {
			s.floor[roomID] = append(entries[:index], entries[index+1:]...)
			break
		}
	}

	if entry.Point != nil {
		entry.Point.Present = false
		entry.Point.EmptySince = time.Now()
	}
}

func (s *State) findInInventory(player *Player, typed string) *Item {
	for _, item := range player.Inventory {
		if matches(typed, item.ID, s.world.items[item.ID].Name) {
			return item
		}
	}

	return nil
}

func (s *State) take(player *Player, typed string) Result {
	room := s.world.rooms[player.Room]

	for _, entry := range s.floor[room.ID] {
		id := entry.Item.ID

		if !matches(typed, id, s.world.items[id].Name) {
			continue
		}

		if holds(player, id) {
			return reply(protocol.ErrAlreadyHaveItem)
		}

		s.removeFloor(room.ID, entry)
		player.Inventory = append(player.Inventory, entry.Item)

		logger.Info("item_taken", logger.Fields{
			"player": player.Username, "item": id, "serial": entry.Item.Serial, "room": room.ID,
		})

		return Result{
			Reply:  "OK taken=" + id,
			Events: []Event{s.toRoom(room.ID, "EVT ROOM ITEM TAKE "+player.Username+" "+id, player.Username)},
		}
	}

	return reply(protocol.ErrItemNotFound)
}

func (s *State) drop(player *Player, typed string) Result {
	item := s.findInInventory(player, typed)
	if item == nil {
		return reply(protocol.ErrItemNotInInv)
	}

	room := s.world.rooms[player.Room]

	player.Inventory = remove(player.Inventory, item)
	s.addFloor(room.ID, &floorItem{Item: item, Expires: time.Now().Add(DropLifetime)})

	logger.Info("item_dropped", logger.Fields{
		"player": player.Username, "item": item.ID, "serial": item.Serial, "room": room.ID,
	})

	return Result{
		Reply:  "OK dropped=" + item.ID,
		Events: []Event{s.toRoom(room.ID, "EVT ROOM ITEM DROP "+player.Username+" "+item.ID, player.Username)},
	}
}

func inventoryIDs(player *Player) []string {
	ids := make([]string, 0, len(player.Inventory))

	for _, item := range player.Inventory {
		ids = append(ids, item.ID)
	}

	return ids
}

type useResponse struct {
	Used  string `json:"used"`
	HP    int    `json:"hp"`
	MaxHP int    `json:"max_hp"`
}

// use heals the player and hands over the item's content, if any; during a fight the enemy then hits back.
func (s *State) use(player *Player, typed string) Result {
	item := s.findInInventory(player, typed)
	if item == nil {
		return reply(protocol.ErrItemNotInInv)
	}

	def := s.world.items[item.ID]
	if def.Heal <= 0 && def.Contains == "" {
		return reply(protocol.ErrItemNotUsable)
	}

	player.HP += def.Heal
	if player.HP > player.MaxHP {
		player.HP = player.MaxHP
	}

	s.consumeItem(player, item.ID, "used")

	logger.Info("item_used", logger.Fields{
		"player": player.Username, "item": item.ID, "hp": player.HP,
	})

	if def.Contains != "" {
		s.grant(player, def.Contains)
	}

	result := Result{}

	if npc := s.currentEnemy(player); npc != nil {
		_, _, result.Events = s.enemyTurn(player, npc)
	}

	result.Reply = okJSON(useResponse{Used: item.ID, HP: player.HP, MaxHP: player.MaxHP})

	return result
}

// consumeItem removes the player's item (used or delivered): the instance ends.
func (s *State) consumeItem(player *Player, id string, reason string) {
	for _, item := range player.Inventory {
		if item.ID == id {
			player.Inventory = remove(player.Inventory, item)
			s.deleteItem(item, reason)

			return
		}
	}
}

// grant gives a reward (quest reward, enemy drop, content of a used item). Nothing is given if the
// player already carries that item. The item never exists more than there are players: a new
// instance is created while there is room for one, else a lying instance is handed over
// (there is always one: every other player carries at most one).
func (s *State) grant(player *Player, id string) {
	if holds(player, id) {
		logger.Info("item_grant_skipped", logger.Fields{"player": player.Username, "item": id})
		return
	}

	var item *Item

	if s.total(id) < len(s.players) {
		item = s.mint(id, "reward")
	} else {
		room, entry := s.pickFloor(id)
		if entry == nil {
			logger.Warn("item_grant_failed", logger.Fields{"player": player.Username, "item": id})
			return
		}

		s.removeFloor(room, entry)
		item = entry.Item

		s.queue(s.toRoom(room, "EVT ROOM ITEM GONE "+id, player.Username))
	}

	player.Inventory = append(player.Inventory, item)

	logger.Info("item_received", logger.Fields{"player": player.Username, "item": id, "serial": item.Serial})
}

// pickFloor finds the lying instance to hand over: dropped ones first, then spawn point ones, lowest serial first.
func (s *State) pickFloor(id string) (string, *floorItem) {
	var bestRoom string
	var best *floorItem

	better := func(candidate, current *floorItem) bool {
		if current == nil {
			return true
		}

		if (candidate.Point == nil) != (current.Point == nil) {
			return candidate.Point == nil
		}

		return candidate.Item.Serial < current.Item.Serial
	}

	for roomID, entries := range s.floor {
		for _, entry := range entries {
			if entry.Item.ID == id && better(entry, best) {
				bestRoom, best = roomID, entry
			}
		}
	}

	return bestRoom, best
}

// Tick runs every second: dropped items expire, spawn points refill.
func (s *State) Tick() []Event {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	now := time.Now()

	for roomID, entries := range s.floor {
		for _, entry := range append([]*floorItem{}, entries...) {
			if entry.Point == nil && now.After(entry.Expires) {
				s.removeFloor(roomID, entry)
				s.deleteItem(entry.Item, "expired")
				s.queue(s.toRoom(roomID, "EVT ROOM ITEM GONE "+entry.Item.ID, ""))
			}
		}
	}

	for _, point := range s.spawns {
		if point.Present || now.Sub(point.EmptySince) < ItemRespawnDelay || s.total(point.ItemID) >= len(s.players) {
			continue
		}

		s.fillSpawnPoint(point, "spawn")
		s.queue(s.toRoom(point.Room, "EVT ROOM ITEM SPAWN "+point.ItemID, ""))
	}

	return s.flush()
}

func (s *State) fillSpawnPoint(point *spawnPoint, reason string) {
	s.addFloor(point.Room, &floorItem{Item: s.mint(point.ItemID, reason), Point: point})
	point.Present = true
}
