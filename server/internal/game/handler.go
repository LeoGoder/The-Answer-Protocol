package game

import (
	"encoding/json"
	"fmt"
	"sort"
	"strings"

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

func (s *State) Handle(username string, cmd protocol.Command) string {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	switch cmd.Name {
	case "LOOK":
		return s.look(username)
	case "MOVE":
		return s.move(username, cmd.Args[0])
	case "CHAT":
		return "OK"
	case "WHO":
		return fmt.Sprintf("OK players=%d", len(s.players))
	case "GROUP":
		return handleGroup(cmd.Args)
	case "TAKE":
		return "ERR 404 ITEM_NOT_FOUND"
	case "DROP":
		return "ERR 404 ITEM_NOT_IN_INVENTORY"
	case "INVENTORY":
		return "OK []"
	case "TALK", "ATTACK", "QUEST":
		return "ERR 404 NPC_NOT_FOUND"
	case "STATUS":
		return `OK {"hp":100,"max_hp":100,"status":"healthy"}`
	case "QUESTS":
		return "OK []"
	}
	return ""
}

func (s *State) look(username string) string {
	player, ok := s.players[username]
	if !ok {
		return protocol.ErrNotAuthenticated
	}
	room, ok := s.rooms[player.CurrentRoom]
	if !ok {
		return "ERR 500 INTERNAL_ERROR"
	}

	players := make([]string, 0)
	for _, p := range s.players {
		if p.CurrentRoom == player.CurrentRoom {
			players = append(players, p.Username)
		}
	}
	sort.Strings(players)

	response := lookResponse{
		Room: roomView{
			ID:          room.ID,
			Name:        room.Name,
			Description: room.Description,
			Exits:       room.Exits,
		},
		Players: players,
		Items:   room.Items,
		NPCs:    room.NPCs,
	}

	data, err := json.Marshal(response)
	if err != nil {
		return "ERR 500 INTERNAL_ERROR"
	}
	return "OK " + string(data)
}

func (s *State) move(username, direction string) string {
	player, ok := s.players[username]
	if !ok {
		return protocol.ErrNotAuthenticated
	}
	room, ok := s.rooms[player.CurrentRoom]
	if !ok {
		return "ERR 500 INTERNAL_ERROR"
	}

	target, ok := room.Exits[strings.ToLower(direction)]
	if !ok {
		return "ERR 301 NO_EXIT"
	}
	if _, ok := s.rooms[target]; !ok {
		return "ERR 301 NO_EXIT"
	}

	player.CurrentRoom = target
	return "OK room=" + target
}

func handleGroup(args []string) string {
	switch strings.ToUpper(args[0]) {
	case "CREATE":
		return "OK group=group.default"
	case "INVITE", "LEAVE":
		return "OK"
	case "JOIN":
		return "OK group=group.default"
	}
	return ""
}
