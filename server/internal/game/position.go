package game

import (
	"encoding/json"
	"strconv"

	"tap/server/internal/protocol"
)

// Positions inside a room: 0..PosMax on both axes, (0, 0) is the north-west corner of the floor.
const PosMax = 1000

// position stores where the player stands in their room (POSITION <x> <y>).
func (s *State) position(player *Player, args []string) Result {
	x, errX := strconv.Atoi(args[0])
	y, errY := strconv.Atoi(args[1])

	if errX != nil || errY != nil || x < 0 || x > PosMax || y < 0 || y > PosMax {
		return reply(protocol.ErrBadRequest)
	}

	player.X = x
	player.Y = y
	player.PosOK = true
	player.WantsPositions = true

	return reply(protocol.OK)
}

// PositionEvents builds one "EVT ROOM POSITION {...}" per room for players who sent POSITION (PositionRate per second).
func (s *State) PositionEvents() []Event {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	positions := make(map[string]map[string][2]int)
	listeners := make(map[string][]string)

	for _, player := range s.players {
		if positions[player.Room] == nil {
			positions[player.Room] = make(map[string][2]int)
		}

		if player.PosOK {
			positions[player.Room][player.Username] = [2]int{player.X, player.Y}
		}

		if player.WantsPositions {
			listeners[player.Room] = append(listeners[player.Room], player.Username)
		}
	}

	events := make([]Event, 0, len(listeners))

	for roomID, to := range listeners {
		data, err := json.Marshal(positions[roomID])
		if err != nil {
			continue
		}

		events = append(events, Event{To: to, Line: "EVT ROOM POSITION " + string(data)})
	}

	return events
}
