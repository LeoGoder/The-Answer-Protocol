package game

import (
	"encoding/json"
	"fmt"
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
	Room    roomView `json:"room"`
	Players []string `json:"players"`
	Items   []string `json:"items"`
	NPCs    []string `json:"npcs"`
}

// Handle locks once: never call an exported method below (deadlock).
func (s *State) Handle(username string, cmd protocol.Command) Result {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	player, exists := s.players[username]
	if !exists {
		return reply(protocol.ErrNotAuthenticated)
	}

	switch cmd.Name {
	case "LOOK":
		return reply(s.look(player))
	case "MOVE":
		return s.move(player, cmd.Args[0])
	case "WHO":
		return reply(fmt.Sprintf("OK players=%d", len(s.players)))
	case "CHAT":
		return s.chat(player, cmd.Args)
	case "GROUP":
		return s.group(player, cmd.Args)
	case "TAKE":
		return s.take(player, targetName(cmd.Args))
	case "DROP":
		return s.drop(player, targetName(cmd.Args))
	case "INVENTORY":
		return reply(s.inventory(player))
	case "USE":
		return s.use(player, targetName(cmd.Args))
	case "TALK":
		return reply(s.talk(player, targetName(cmd.Args)))
	case "ATTACK":
		return s.attack(player, targetName(cmd.Args))
	case "STATUS":
		return reply(s.status(player))
	case "QUEST":
		return reply(s.quest(player, targetName(cmd.Args)))
	case "QUESTS":
		return reply(s.questLog(player))
	case "ALIVE":
		return reply("Alive")
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

func (s *State) look(player *Player) string {
	room := s.world.rooms[player.Room]

	return okJSON(lookResponse{
		Room: roomView{
			ID:          room.ID,
			Name:        room.Name,
			Description: room.Description,
			Exits:       room.Exits,
		},
		Players: s.playersIn(room.ID),
		Items:   room.Items,
		NPCs:    s.aliveNPCsIn(room.ID),
	})
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
