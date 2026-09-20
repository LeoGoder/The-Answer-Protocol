package dispatcher

import "tap/server/internal/protocol"

type Target string

const (
	Session Target = "session"
	Game    Target = "game"
	Invalid Target = "invalid"
)

func Route(cmd protocol.Command) Target {
	switch cmd.Name {
	case "CONNECT", "QUIT":
		return Session

	case "LOOK", "MOVE", "CHAT", "WHO", "GROUP",
		"TAKE", "DROP", "INVENTORY", "TALK",
		"ATTACK", "STATUS", "QUEST", "QUESTS":
		return Game

	default:
		return Invalid
	}
}