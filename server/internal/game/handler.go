package game

import (
	"encoding/json"
	"fmt"
	"sort"
	"strings"

	"tap/server/internal/logger"
	"tap/server/internal/protocol"
)

type roomView struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	Description string            `json:"description"`
	Exits       map[string]string `json:"exits"`
}

type lookResponse struct {
	Room    roomView          `json:"room"`
	Players []string          `json:"players"`
	Items   []string          `json:"items"`
	NPCs    []string          `json:"npcs"`
	Names   map[string]string `json:"names"`
}

// Handle locks once: never call an exported method below (deadlock).
func (s *State) Handle(username string, cmd protocol.Command) Result {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	result := s.handle(username, cmd)
	result.Events = append(result.Events, s.flush()...)

	return result
}

func (s *State) handle(username string, cmd protocol.Command) Result {
	player, exists := s.players[username]
	if !exists {
		return reply(protocol.ErrNotAuthenticated)
	}

	// Item and NPC names can contain spaces: "TAKE Pomme de terre".
	target := strings.Join(cmd.Args, " ")

	switch cmd.Name {
	case "LOOK":
		return s.look(player)
	case "MOVE":
		return s.move(player, cmd.Args[0])
	case "WHO":
		return reply(fmt.Sprintf("OK players=%d", len(s.players)))
	case "CHAT":
		return s.chat(player, cmd.Args)
	case "GROUP":
		return s.group(player, cmd.Args)
	case "TAKE":
		return s.take(player, target)
	case "DROP":
		return s.drop(player, target)
	case "INVENTORY":
		return reply(okJSON(inventoryIDs(player)))
	case "USE":
		return s.use(player, target)
	case "TALK":
		return s.talk(player, target)
	case "ATTACK":
		return s.attack(player, target)
	case "STATUS":
		return s.status(player)
	case "QUEST":
		return s.quest(player, target)
	case "QUESTS":
		return s.questLog(player)
	case "ALIVE":
		return reply("OK Alive")
	case "POSITION":
		return s.position(player, cmd.Args)
	}

	return reply(protocol.ErrUnknownCommand)
}

func reply(line string) Result {
	return Result{Reply: line}
}

func okJSON(value interface{}) string {
	data, err := json.Marshal(value)
	if err != nil {
		logger.Error("json_encode_failed", logger.Fields{"error": err.Error()})
		return protocol.ErrInternal
	}

	return "OK " + string(data)
}

func (s *State) look(player *Player) Result {
	room := s.world.rooms[player.Room]
	npcs := s.aliveNPCsIn(room.ID)
	items := s.floorIDs(room.ID)

	// The RFC only sends IDs: names let the clients show "Chef Gusto" instead of "chef".
	names := make(map[string]string)

	for _, id := range items {
		names[id] = s.world.items[id].Name
	}

	for _, item := range player.Inventory {
		names[item.ID] = s.world.items[item.ID].Name
	}

	for _, id := range npcs {
		names[id] = s.world.npcs[id].Name
	}

	for _, target := range room.Exits {
		names[target] = s.world.rooms[target].Name
	}

	return reply(okJSON(lookResponse{
		Room: roomView{
			ID:          room.ID,
			Name:        room.Name,
			Description: room.Description,
			Exits:       room.Exits,
		},
		Players: s.playersIn(room.ID),
		Items:   items,
		NPCs:    npcs,
		Names:   names,
	}))
}

func (s *State) playersIn(roomID string) []string {
	names := make([]string, 0)

	for _, player := range s.players {
		if player.Room == roomID {
			names = append(names, player.Username)
		}
	}

	sort.Strings(names)

	return names
}

func (s *State) move(player *Player, direction string) Result {
	room := s.world.rooms[player.Room]

	target, exists := room.Exits[strings.ToLower(direction)]
	if !exists {
		return reply(protocol.ErrNoExit)
	}

	result := Result{Reply: "OK room=" + target}

	if player.Target != "" {
		logger.Info("combat_left", logger.Fields{
			"player": player.Username, "npc": player.Target, "reason": "moved",
		})
		player.Target = ""
	}

	result.Events = s.changeRoom(player, target)

	logger.Info("player_moved", logger.Fields{
		"player": player.Username, "from": room.ID, "to": target,
	})

	return result
}

// changeRoom returns the LEAVE / ENTER events for the others.
func (s *State) changeRoom(player *Player, target string) []Event {
	oldRoom := player.Room
	player.Room = target
	player.PosOK = false

	return []Event{
		s.toRoom(oldRoom, "EVT ROOM PRESENCE LEAVE "+player.Username, player.Username),
		s.toRoom(target, "EVT ROOM PRESENCE ENTER "+player.Username, player.Username),
	}
}

func (s *State) chat(player *Player, args []string) Result {
	scope := strings.ToUpper(args[0])
	message := strings.Join(args[1:], " ")

	logger.Info("chat", logger.Fields{
		"player": player.Username, "scope": scope, "message": message,
	})

	switch scope {
	case "GLOBAL":
		line := "EVT GLOBAL CHAT " + player.Username + " " + message
		return Result{Reply: protocol.OK, Events: []Event{s.toAll(line, "")}}

	case "ROOM":
		line := "EVT ROOM CHAT " + player.Username + " " + message
		return Result{Reply: protocol.OK, Events: []Event{s.toRoom(player.Room, line, "")}}

	case "GROUP":
		if player.Group == "" {
			return reply(protocol.ErrNotInGroup)
		}

		line := "EVT GROUP CHAT " + player.Username + " " + message
		return Result{Reply: protocol.OK, Events: []Event{s.toGroup(player.Group, line, "")}}
	}

	return reply(protocol.ErrBadRequest)
}
