package game

import (
	"fmt"
	"math/rand"
	"strings"
	"sync"
	"time"

	"tap/server/internal/logger"
)

const (
	StartHP      = 100
	StartAttack  = 10
	StartDefense = 2
)

// Event is one line ("EVT ...") to send to some players.
type Event struct {
	To   []string
	Line string
}

// Result is the reply for the sender plus the events for the others.
type Result struct {
	Reply  string
	Events []Event
}

// queue keeps an event raised deep in the game logic; Handle and Tick send it.
func (s *State) queue(event Event) {
	s.pending = append(s.pending, event)
}

func (s *State) flush() []Event {
	events := s.pending
	s.pending = nil

	return events
}

type questProgress struct {
	QuestID   string
	Completed bool
	Defeated  bool // defeat quests: the target was defeated
}

type Player struct {
	Username       string
	Room           string
	HP             int
	MaxHP          int
	Attack         int
	Defense        int
	Inventory      []*Item // at most one item of each ID
	Target         string  // NPC being fought, "" if none
	Group          string  // "" if none
	Quests         []*questProgress
	X, Y           int  // position inside the room, 0..PosMax (POSITION)
	PosOK          bool // X, Y were sent in the current room
	WantsPositions bool // sent POSITION at least once: receives EVT ROOM POSITION
}

type npcState struct {
	Def       *NPCDef
	Room      string
	HP        int
	RespawnAt time.Time
	TalkIndex int
}

type Group struct {
	ID      string
	Members []string
	Invited map[string]bool
}

// Exported methods take the mutex; unexported ones expect it held.
type State struct {
	mutex sync.Mutex
	world *world

	floor       map[string][]*floorItem // room ID -> items lying there
	spawns      []*spawnPoint
	nextSerial  map[string]int // item ID -> serial of its next instance
	pending     []Event        // raised by the game logic, sent by Handle / Tick
	npcs        map[string]*npcState
	players     map[string]*Player
	groups      map[string]*Group
	nextGroupID int
	random      *rand.Rand
}

func newState(w *world) *State {
	state := &State{
		world:       w,
		floor:       make(map[string][]*floorItem),
		nextSerial:  make(map[string]int),
		npcs:        make(map[string]*npcState),
		players:     make(map[string]*Player),
		groups:      make(map[string]*Group),
		nextGroupID: 1,
		random:      rand.New(rand.NewSource(time.Now().UnixNano())),
	}

	for _, room := range w.rooms {
		for _, itemID := range room.Items {
			point := &spawnPoint{Room: room.ID, ItemID: itemID}
			state.spawns = append(state.spawns, point)
			state.fillSpawnPoint(point, "world")
		}

		for _, npcID := range room.NPCs {
			def := w.npcs[npcID]
			state.npcs[npcID] = &npcState{Def: def, Room: room.ID, HP: def.HP}
		}
	}

	return state
}

// Join returns false if the name is taken (check and add under one lock).
func (s *State) Join(username string) ([]Event, bool) {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	if _, exists := s.players[username]; exists {
		return nil, false
	}

	s.players[username] = &Player{
		Username:  username,
		Room:      s.world.startRoom,
		HP:        StartHP,
		MaxHP:     StartHP,
		Attack:    StartAttack,
		Defense:   StartDefense,
		Inventory: []*Item{},
	}

	events := []Event{
		s.toRoom(s.world.startRoom, "EVT ROOM PRESENCE ENTER "+username, username),
		s.statsEvent(),
	}

	return events, true
}

// Leave removes the player before building the LEAVE events; what they carry goes with them.
func (s *State) Leave(username string) []Event {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	player, exists := s.players[username]
	if !exists {
		return nil
	}

	var events []Event

	if player.Group != "" {
		events = append(events, s.leaveGroup(player)...)
	}

	if len(player.Inventory) > 0 {
		logger.Info("items_removed_on_disconnect", logger.Fields{
			"player": username, "items": inventoryIDs(player),
		})

		for _, item := range player.Inventory {
			s.deleteItem(item, "owner_disconnected")
		}
	}

	delete(s.players, username)

	events = append(events,
		s.toRoom(player.Room, "EVT ROOM PRESENCE LEAVE "+username, ""),
		s.statsEvent(),
	)

	return events
}

// Recipient helpers: "exclude" is a player left out, or "" for nobody.

func (s *State) toRoom(roomID string, line string, exclude string) Event {
	var to []string

	for _, player := range s.players {
		if player.Room == roomID && player.Username != exclude {
			to = append(to, player.Username)
		}
	}

	return Event{To: to, Line: line}
}

func (s *State) toGroup(groupID string, line string, exclude string) Event {
	var to []string

	if group, exists := s.groups[groupID]; exists {
		for _, member := range group.Members {
			if member != exclude {
				to = append(to, member)
			}
		}
	}

	return Event{To: to, Line: line}
}

func (s *State) toAll(line string, exclude string) Event {
	var to []string

	for name := range s.players {
		if name != exclude {
			to = append(to, name)
		}
	}

	return Event{To: to, Line: line}
}

func (s *State) statsEvent() Event {
	return s.toAll(fmt.Sprintf("EVT STATS players=%d", len(s.players)), "")
}

// normalize makes "  Object   ONE " equal to "object one".
func normalize(text string) string {
	return strings.ToLower(strings.Join(strings.Fields(text), " "))
}

// matches accepts the ID ("beef_wellington") or the display name ("Beef Wellington"), in any case.
func matches(typed string, id string, name string) bool {
	typed = normalize(typed)

	return typed == strings.ToLower(id) || typed == normalize(name)
}

// remove removes the first element equal to value.
func remove[T comparable](list []T, value T) []T {
	for index, current := range list {
		if current == value {
			return append(list[:index], list[index+1:]...)
		}
	}

	return list
}
