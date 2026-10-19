package protocol

import (
	"fmt"
	"strings"
)

// Validate checks arguments only; unknown commands pass to the game.
func Validate(cmd Command) error {
	switch cmd.Name {

	case "CONNECT":
		if len(cmd.Args) != 1 {
			return fmt.Errorf("CONNECT requires one argument")
		}

		return ValidateUsername(cmd.Args[0])

	case "MOVE":
		if len(cmd.Args) != 1 {
			return fmt.Errorf("MOVE requires one argument")
		}

	case "QUIT", "LOOK", "WHO", "INVENTORY", "STATUS", "QUESTS":
		if len(cmd.Args) != 0 {
			return fmt.Errorf("%s does not accept arguments", cmd.Name)
		}

	case "CHAT":
		if len(cmd.Args) < 2 {
			return fmt.Errorf("CHAT requires scope and message")
		}

		scope := strings.ToUpper(cmd.Args[0])

		if scope != "GLOBAL" && scope != "ROOM" && scope != "GROUP" {
			return fmt.Errorf("invalid CHAT scope")
		}

	case "GROUP":
		return validateGroup(cmd.Args)

	// At least one word: names can contain spaces ("Object 1").
	case "TAKE", "DROP", "TALK", "ATTACK", "QUEST", "USE":
		if len(cmd.Args) == 0 {
			return fmt.Errorf("%s requires a target", cmd.Name)
		}
	}

	return nil
}

func validateGroup(args []string) error {
	if len(args) == 0 {
		return fmt.Errorf("GROUP requires an action")
	}

	action := strings.ToUpper(args[0])

	switch action {

	case "CREATE", "LEAVE":
		if len(args) != 1 {
			return fmt.Errorf("GROUP %s accepts no argument", action)
		}

	case "INVITE", "JOIN":
		if len(args) != 2 {
			return fmt.Errorf("GROUP %s requires one username", action)
		}

	default:
		return fmt.Errorf("unknown GROUP action")
	}

	return nil
}
