package game

import (
	"tap/server/internal/logger"
	"tap/server/internal/protocol"
)

// An item ID is in exactly one list (room or inventory): no duplicates.

func (s *State) findItemIn(list []string, typed string) string {
	for _, id := range list {
		if matches(typed, id, s.world.items[id].Name) {
			return id
		}
	}

	return ""
}

func (s *State) take(player *Player, typed string) Result {
	room := s.world.rooms[player.Room]

	id := s.findItemIn(room.Items, typed)
	if id == "" {
		return reply(protocol.ErrItemNotFound)
	}

	room.Items, _ = removeString(room.Items, id)
	player.Inventory = append(player.Inventory, id)

	logger.Info("item_taken", logger.Fields{
		"player": player.Username, "item": id, "room": room.ID,
	})

	return reply("OK taken=" + id)
}

func (s *State) drop(player *Player, typed string) Result {
	id := s.findItemIn(player.Inventory, typed)
	if id == "" {
		return reply(protocol.ErrItemNotInInv)
	}

	room := s.world.rooms[player.Room]

	player.Inventory, _ = removeString(player.Inventory, id)
	room.Items = append(room.Items, id)

	logger.Info("item_dropped", logger.Fields{
		"player": player.Username, "item": id, "room": room.ID,
	})

	return reply("OK dropped=" + id)
}

func (s *State) inventory(player *Player) string {
	return okJSON(player.Inventory)
}

type useResponse struct {
	Used  string `json:"used"`
	HP    int    `json:"hp"`
	MaxHP int    `json:"max_hp"`
}

// use heals the player; during a fight the enemy then hits back.
func (s *State) use(player *Player, typed string) Result {
	id := s.findItemIn(player.Inventory, typed)
	if id == "" {
		return reply(protocol.ErrItemNotInInv)
	}

	item := s.world.items[id]
	if item.Heal <= 0 {
		return reply(protocol.ErrItemNotUsable)
	}

	player.HP += item.Heal
	if player.HP > player.MaxHP {
		player.HP = player.MaxHP
	}

	s.consumeItem(player, id)

	logger.Info("item_used", logger.Fields{
		"player": player.Username, "item": id, "hp": player.HP,
	})

	result := Result{}

	if npc := s.currentEnemy(player); npc != nil {
		_, _, result.Events = s.enemyTurn(player, npc)
	}

	result.Reply = okJSON(useResponse{Used: id, HP: player.HP, MaxHP: player.MaxHP})

	return result
}

// consumeItem puts a used or delivered item back where it spawns.
func (s *State) consumeItem(player *Player, id string) {
	player.Inventory, _ = removeString(player.Inventory, id)

	origin := s.world.rooms[s.world.itemRoom[id]]
	origin.Items = append(origin.Items, id)

	logger.Info("item_respawned", logger.Fields{"item": id, "room": origin.ID})
}
