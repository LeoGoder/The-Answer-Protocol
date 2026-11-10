package game

import (
	"encoding/json"
	"fmt"
	"os"
)

type Room struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	Description string            `json:"description"`
	Exits       map[string]string `json:"exits"`
	Items       []string          `json:"items"` // changes with TAKE / DROP
	NPCs        []string          `json:"npcs"`
}

type ItemDef struct {
	ID          string `json:"id"`
	Name        string `json:"name"`
	Description string `json:"description"`
	Heal        int    `json:"heal"` // > 0: the item can be used (USE)
}

type NPCDef struct {
	ID          string   `json:"id"`
	Name        string   `json:"name"`
	Description string   `json:"description"`
	Dialogue    []string `json:"dialogue"`
	Hostile     bool     `json:"hostile"` // enemy: can be attacked
	HP          int      `json:"hp"`
	Attack      int      `json:"attack"`
	Defense     int      `json:"defense"`
}

type Reward struct {
	Name   string `json:"name"`
	MaxHP  int    `json:"max_hp"`
	Attack int    `json:"attack"`
}

const (
	QuestFetch  = "fetch"  // target = item to bring to the giver
	QuestDefeat = "defeat" // target = enemy to defeat
)

// The only exit names allowed in world.json.
var directions = map[string]bool{"north": true, "south": true, "east": true, "west": true}

type QuestDef struct {
	ID          string `json:"id"`
	Type        string `json:"type"`
	Giver       string `json:"giver"`
	Description string `json:"description"`
	Target      string `json:"target"`
	Reward      Reward `json:"reward"`
}

// worldFile has the shape of world.json.
type worldFile struct {
	StartRoom   string     `json:"start_room"`
	RespawnRoom string     `json:"respawn_room"`
	Rooms       []Room     `json:"rooms"`
	Items       []ItemDef  `json:"items"`
	NPCs        []NPCDef   `json:"npcs"`
	Quests      []QuestDef `json:"quests"`
}

// world is the loaded data, indexed by ID.
type world struct {
	startRoom   string
	respawnRoom string
	rooms       map[string]*Room
	items       map[string]*ItemDef
	npcs        map[string]*NPCDef
	quests      map[string]*QuestDef
	questOrder  []string          // quest IDs in file order
	itemRoom    map[string]string // item ID -> room where it spawns
}

func LoadState(path string) (*State, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("cannot load %s: %v", path, err)
	}

	var file worldFile

	if err := json.Unmarshal(data, &file); err != nil {
		return nil, fmt.Errorf("invalid %s: %v", path, err)
	}

	w, err := buildWorld(&file)
	if err != nil {
		return nil, err
	}

	if err := w.validate(); err != nil {
		return nil, err
	}

	return newState(w), nil
}

// buildWorld puts every list into a map and refuses duplicate IDs.
func buildWorld(file *worldFile) (*world, error) {
	w := &world{
		startRoom:   file.StartRoom,
		respawnRoom: file.RespawnRoom,
		rooms:       make(map[string]*Room),
		items:       make(map[string]*ItemDef),
		npcs:        make(map[string]*NPCDef),
		quests:      make(map[string]*QuestDef),
		itemRoom:    make(map[string]string),
	}

	if w.respawnRoom == "" {
		w.respawnRoom = w.startRoom
	}

	for i := range file.Rooms {
		room := &file.Rooms[i]
		if room.ID == "" || w.rooms[room.ID] != nil {
			return nil, fmt.Errorf("room %q: missing or duplicate id", room.ID)
		}

		if room.Exits == nil {
			room.Exits = map[string]string{}
		}

		if room.Items == nil {
			room.Items = []string{}
		}

		if room.NPCs == nil {
			room.NPCs = []string{}
		}

		w.rooms[room.ID] = room
	}

	for i := range file.Items {
		item := &file.Items[i]
		if item.ID == "" || w.items[item.ID] != nil {
			return nil, fmt.Errorf("item %q: missing or duplicate id", item.ID)
		}
		w.items[item.ID] = item
	}

	for i := range file.NPCs {
		npc := &file.NPCs[i]
		if npc.ID == "" || w.npcs[npc.ID] != nil {
			return nil, fmt.Errorf("npc %q: missing or duplicate id", npc.ID)
		}
		w.npcs[npc.ID] = npc
	}

	for i := range file.Quests {
		quest := &file.Quests[i]
		if quest.ID == "" || w.quests[quest.ID] != nil {
			return nil, fmt.Errorf("quest %q: missing or duplicate id", quest.ID)
		}
		w.quests[quest.ID] = quest
		w.questOrder = append(w.questOrder, quest.ID)
	}

	return w, nil
}

// validate checks every reference: a broken world.json stops the server.
func (w *world) validate() error {
	if len(w.rooms) == 0 {
		return fmt.Errorf("world has no rooms")
	}

	if w.rooms[w.startRoom] == nil {
		return fmt.Errorf("start_room %q does not exist", w.startRoom)
	}

	if w.rooms[w.respawnRoom] == nil {
		return fmt.Errorf("respawn_room %q does not exist", w.respawnRoom)
	}

	if err := w.validateNames(); err != nil {
		return err
	}

	if err := w.validateRooms(); err != nil {
		return err
	}

	return w.validateQuests()
}

func (w *world) validateNames() error {
	names := make(map[string]string)

	for id, item := range w.items {
		if item.Name == "" {
			return fmt.Errorf("item %s has no name", id)
		}

		// Two equal names would make "TAKE <name>" ambiguous.
		if other, exists := names[normalize(item.Name)]; exists {
			return fmt.Errorf("%s and %s have the same name", other, id)
		}
		names[normalize(item.Name)] = id
	}

	for id, npc := range w.npcs {
		if npc.Name == "" || len(npc.Dialogue) == 0 {
			return fmt.Errorf("npc %s needs a name and a dialogue", id)
		}

		if npc.Hostile && (npc.HP <= 0 || npc.Attack <= 0) {
			return fmt.Errorf("enemy %s needs hp and attack above 0", id)
		}

		if other, exists := names[normalize(npc.Name)]; exists {
			return fmt.Errorf("%s and %s have the same name", other, id)
		}
		names[normalize(npc.Name)] = id
	}

	return nil
}

func (w *world) validateRooms() error {
	// Items and NPCs are unique: each one is placed in one room only.
	placed := make(map[string]string)

	for id, room := range w.rooms {
		for direction, target := range room.Exits {
			if !directions[direction] {
				return fmt.Errorf("room %s: invalid exit %q", id, direction)
			}

			if w.rooms[target] == nil {
				return fmt.Errorf("room %s: exit %s leads to unknown room %s", id, direction, target)
			}
		}

		for _, itemID := range room.Items {
			if w.items[itemID] == nil {
				return fmt.Errorf("room %s: unknown item %s", id, itemID)
			}
			w.itemRoom[itemID] = id
		}

		for _, npcID := range room.NPCs {
			if w.npcs[npcID] == nil {
				return fmt.Errorf("room %s: unknown npc %s", id, npcID)
			}

			if id == w.respawnRoom && w.npcs[npcID].Hostile {
				return fmt.Errorf("respawn room %s contains enemy %s", id, npcID)
			}
		}

		for _, resource := range append(append([]string{}, room.Items...), room.NPCs...) {
			if other, exists := placed[resource]; exists {
				return fmt.Errorf("%s is placed twice (%s and %s)", resource, other, id)
			}
			placed[resource] = id
		}
	}

	return nil
}

func (w *world) validateQuests() error {
	for id, quest := range w.quests {
		if quest.Description == "" || quest.Reward.Name == "" {
			return fmt.Errorf("quest %s needs a description and a reward name", id)
		}

		if w.npcs[quest.Giver] == nil {
			return fmt.Errorf("quest %s: unknown giver %s", id, quest.Giver)
		}

		switch quest.Type {
		case QuestFetch:
			if _, placed := w.itemRoom[quest.Target]; !placed {
				return fmt.Errorf("fetch quest %s: item %q is not placed in any room", id, quest.Target)
			}

		case QuestDefeat:
			target := w.npcs[quest.Target]
			if target == nil || !target.Hostile {
				return fmt.Errorf("defeat quest %s: target %q is not an enemy", id, quest.Target)
			}

		default:
			return fmt.Errorf("quest %s: unknown type %q", id, quest.Type)
		}
	}

	return nil
}
