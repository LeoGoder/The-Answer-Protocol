package game

import (
	"encoding/json"
	"fmt"
	"os"
	"sync"
)

type Room struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	Description string            `json:"description"`
	Exits       map[string]string `json:"exits"`
	Items       []string          `json:"items"`
	NPCs        []string          `json:"npcs"`
}

type Player struct {
	Username    string
	CurrentRoom string
	HP          int
	MaxHP       int
}

type worldFile struct {
	StartRoom string           `json:"start_room"`
	Rooms     map[string]*Room `json:"rooms"`
}

type State struct {
	mutex     sync.Mutex
	startRoom string
	rooms     map[string]*Room
	players   map[string]*Player
}

func LoadState() (*State, error) {
	var data []byte
	var err error

	for _, path := range []string{
		"data/world.json",
		"server/data/world.json",
		"../../data/world.json",
	} {
		data, err = os.ReadFile(path)
		if err == nil {
			break
		}
	}

	if err != nil {
		return nil, fmt.Errorf("cannot load world.json: %v", err)
	}

	var file worldFile

	if err := json.Unmarshal(data, &file); err != nil {
		return nil, fmt.Errorf("invalid world.json: %v", err)
	}

	if file.StartRoom == "" {
		return nil, fmt.Errorf("world has no start_room")
	}

	if len(file.Rooms) == 0 {
		return nil, fmt.Errorf("world has no rooms")
	}

	if _, exists := file.Rooms[file.StartRoom]; !exists {
		return nil, fmt.Errorf(
			"start room %s does not exist",
			file.StartRoom,
		)
	}

	for id, room := range file.Rooms {
		if room == nil {
			return nil, fmt.Errorf("room %s is null", id)
		}

		room.ID = id

		if room.Exits == nil {
			room.Exits = map[string]string{}
		}

		if room.Items == nil {
			room.Items = []string{}
		}

		if room.NPCs == nil {
			room.NPCs = []string{}
		}

		for direction, target := range room.Exits {
			if _, exists := file.Rooms[target]; !exists {
				return nil, fmt.Errorf(
					"room %s exit %s points to unknown room %s",
					id,
					direction,
					target,
				)
			}
		}
	}

	return &State{
		startRoom: file.StartRoom,
		rooms:     file.Rooms,
		players:   make(map[string]*Player),
	}, nil
}

func (s *State) AddPlayer(username string) bool {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	if _, exists := s.players[username]; exists {
		return false
	}

	s.players[username] = &Player{
		Username:    username,
		CurrentRoom: s.startRoom,
		HP:          100,
		MaxHP:       100,
	}

	return true
}

func (s *State) RemovePlayer(username string) {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	delete(s.players, username)
}

func (s *State) PlayerRoom(username string) (string, bool) {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	player, exists := s.players[username]
	if !exists {
		return "", false
	}

	return player.CurrentRoom, true
}

func (s *State) PlayersInRoom(roomID string) []string {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	usernames := make([]string, 0)

	for _, player := range s.players {
		if player.CurrentRoom == roomID {
			usernames = append(usernames, player.Username)
		}
	}

	return usernames
}

func (s *State) PlayersInSameRoom(username string) []string {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	player, exists := s.players[username]
	if !exists {
		return nil
	}

	usernames := make([]string, 0)

	for _, otherPlayer := range s.players {
		if otherPlayer.CurrentRoom == player.CurrentRoom {
			usernames = append(
				usernames,
				otherPlayer.Username,
			)
		}
	}

	return usernames
}
