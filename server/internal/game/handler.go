package game

import (
	"fmt"
	"strings"

	"tap/server/internal/protocol"
)

func Handle(cmd protocol.Command, playerCount int) string {
	switch cmd.Name {

	case "LOOK":
		return `OK {"room":{"id":"loc.square","name":"Village Square","description":"A peaceful village square.","exits":{"north":"loc.bakery"}},"players":[],"items":["item.herbs"],"npcs":["npc.baker"]}`

	case "MOVE":
		return "OK room=loc.square"

	case "CHAT":
		return "OK"

	case "WHO":
		return fmt.Sprintf("OK players=%d", playerCount)

	case "GROUP":
		return handleGroup(cmd.Args)

	case "TAKE":
		return "OK taken=item.herbs"

	case "DROP":
		return "OK dropped=item.herbs"

	case "INVENTORY":
		return `OK ["item.herbs"]`

	case "TALK":
		return "OK Welcome to my bakery!"

	case "ATTACK":
		return `OK {"attacker_hp":90,"target_hp":15,"damage":10,"status":"combat"}`

	case "STATUS":
		return `OK {"hp":100,"max_hp":100,"status":"healthy"}`

	case "QUEST":
		return `OK {"quest_id":"fetch_herbs","description":"Bring me 3 healing herbs","reward":"gold_coin","status":"available"}`

	case "QUESTS":
		return `OK [{"quest_id":"fetch_herbs","status":"active","progress":"0/3"}]`
	}

	return ""
}

func handleGroup(args []string) string {
	if len(args) == 0 {
		return ""
	}

	action := strings.ToUpper(args[0])

	switch action {
	case "CREATE":
		return "OK group=group.demo"

	case "INVITE":
		return "OK"

	case "JOIN":
		return "OK group=group.demo"

	case "LEAVE":
		return "OK"
	}

	return ""
}