package protocol

import (
	"fmt"
	"strings"
)

func Validate(cmd Command) error {
	switch cmd.Name {

	case "CONNECT":
		if len(cmd.Args) != 1 {
			return fmt.Errorf("CONNECT requires one username")
		}

	case "QUIT", "LOOK", "WHO", "INVENTORY", "STATUS", "QUESTS":
		if len(cmd.Args) != 0 {
			return fmt.Errorf("%s does not accept arguments", cmd.Name)
		}

	case "MOVE":
		if len(cmd.Args) != 1 {
			return fmt.Errorf("MOVE requires one direction")
		}

	case "CHAT":
		if len(cmd.Args) < 2 {
			return fmt.Errorf("CHAT requires scope and message")
		}

		scope := strings.ToUpper(cmd.Args[0])

		if scope != "GLOBAL" &&
			scope != "ROOM" &&
			scope != "GROUP" {
			return fmt.Errorf("invalid CHAT scope")
		}

	case "GROUP":
		return validateGroup(cmd.Args)

	case "TAKE", "DROP", "TALK", "ATTACK", "QUEST":
		if len(cmd.Args) < 1 {
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