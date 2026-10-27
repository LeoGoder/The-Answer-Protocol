package game

import (
	"fmt"
	"strings"

	"tap/server/internal/logger"
	"tap/server/internal/protocol"
)

// JOIN needs an invitation; an empty group is deleted.

func (s *State) group(player *Player, args []string) Result {
	switch strings.ToUpper(args[0]) {
	case "CREATE":
		return s.createGroup(player)
	case "INVITE":
		return s.inviteToGroup(player, args[1])
	case "JOIN":
		return s.joinGroup(player, args[1])
	case "LEAVE":
		if player.Group == "" {
			return reply(protocol.ErrNotInGroup)
		}

		return Result{Reply: protocol.OK, Events: s.leaveGroup(player)}
	}

	return reply(protocol.ErrBadRequest)
}

func (s *State) createGroup(player *Player) Result {
	if player.Group != "" {
		return reply(protocol.ErrAlreadyInGroup)
	}

	id := fmt.Sprintf("group.%d", s.nextGroupID)
	s.nextGroupID++

	s.groups[id] = &Group{
		ID:      id,
		Members: []string{player.Username},
		Invited: make(map[string]bool),
	}
	player.Group = id

	logger.Info("group_created", logger.Fields{"player": player.Username, "group": id})

	return reply("OK group=" + id)
}

func (s *State) inviteToGroup(player *Player, username string) Result {
	if player.Group == "" {
		return reply(protocol.ErrNotInGroup)
	}

	target, exists := s.players[username]
	if !exists {
		return reply(protocol.ErrPlayerNotFound)
	}

	if target == player {
		return reply(protocol.ErrBadRequest)
	}

	if target.Group != "" {
		return reply(protocol.ErrAlreadyInGroup)
	}

	s.groups[player.Group].Invited[target.Username] = true

	logger.Info("group_invite", logger.Fields{
		"player": player.Username, "invited": target.Username, "group": player.Group,
	})

	return Result{
		Reply:  protocol.OK,
		Events: []Event{{To: []string{target.Username}, Line: "EVT GROUP INVITE " + player.Username}},
	}
}

// joinGroup joins the group of a player (usually the one who sent the invitation).
func (s *State) joinGroup(player *Player, name string) Result {
	if player.Group != "" {
		return reply(protocol.ErrAlreadyInGroup)
	}

	member, online := s.players[name]
	if !online {
		return reply(protocol.ErrPlayerNotFound)
	}

	if member.Group == "" {
		return reply(protocol.ErrNotInGroup)
	}

	group := s.groups[member.Group]

	if !group.Invited[player.Username] {
		return reply(protocol.ErrNotInvited)
	}

	delete(group.Invited, player.Username)
	group.Members = append(group.Members, player.Username)
	player.Group = group.ID

	logger.Info("group_joined", logger.Fields{"player": player.Username, "group": group.ID})

	return Result{
		Reply:  "OK group=" + group.ID,
		Events: []Event{s.toGroup(group.ID, "EVT GROUP JOIN "+player.Username, player.Username)},
	}
}

func (s *State) leaveGroup(player *Player) []Event {
	group := s.groups[player.Group]

	group.Members = removeString(group.Members, player.Username)
	player.Group = ""

	logger.Info("group_left", logger.Fields{"player": player.Username, "group": group.ID})

	event := s.toGroup(group.ID, "EVT GROUP LEAVE "+player.Username, "")

	if len(group.Members) == 0 {
		delete(s.groups, group.ID)
		logger.Info("group_disbanded", logger.Fields{"group": group.ID})
	}

	return []Event{event}
}
