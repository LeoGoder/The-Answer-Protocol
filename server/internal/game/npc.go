package game

import (
	"time"

	"tap/server/internal/logger"
	"tap/server/internal/protocol"
)

// isAlive also respawns a defeated enemy once RespawnAt has passed.
func (s *State) isAlive(npc *npcState) bool {
	if !npc.Def.Hostile || npc.HP > 0 {
		return true
	}

	if time.Now().Before(npc.RespawnAt) {
		return false
	}

	npc.HP = npc.Def.HP
	logger.Info("npc_respawned", logger.Fields{"npc": npc.Def.ID, "room": npc.Room})

	return true
}

func (s *State) aliveNPCsIn(roomID string) []string {
	ids := make([]string, 0)

	for _, id := range s.world.rooms[roomID].NPCs {
		if s.isAlive(s.npcs[id]) {
			ids = append(ids, id)
		}
	}

	return ids
}

// findNPC returns the NPC of the player's room matching what was typed.
func (s *State) findNPC(player *Player, typed string) *npcState {
	for _, id := range s.aliveNPCsIn(player.Room) {
		npc := s.npcs[id]
		if matches(typed, id, npc.Def.Name) {
			return npc
		}
	}

	return nil
}

// talk returns the NPC's dialogue lines one after the other, in a loop.
func (s *State) talk(player *Player, typed string) string {
	npc := s.findNPC(player, typed)
	if npc == nil {
		return protocol.ErrNPCNotFound
	}

	line := npc.Def.Dialogue[npc.TalkIndex%len(npc.Def.Dialogue)]
	npc.TalkIndex++

	logger.Info("npc_talk", logger.Fields{
		"player": player.Username, "npc": npc.Def.ID, "line": line,
	})

	return "OK " + line
}
