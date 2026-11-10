package game

import (
	"time"

	"tap/server/internal/logger"
	"tap/server/internal/protocol"
)

// Everything about NPCs: finding them, TALK, ATTACK and STATUS.
// One ATTACK = one round: player hits, then the enemy hits back; MOVE leaves the fight.

const EnemyRespawnDelay = 30 * time.Second

const (
	StatusCombat   = "combat"
	StatusVictory  = "victory"
	StatusDefeated = "defeated"
)

// Reply to ATTACK, and payload of "EVT ROOM COMBAT <json>".
type combatResponse struct {
	Attacker    string `json:"attacker"`
	Target      string `json:"target"`
	AttackerHP  int    `json:"attacker_hp"`
	TargetHP    int    `json:"target_hp"`
	Damage      int    `json:"damage"`
	DamageTaken int    `json:"damage_taken"`
	Status      string `json:"status"`
	Room        string `json:"room,omitempty"` // set when the player respawned
}

type statusResponse struct {
	HP      int    `json:"hp"`
	MaxHP   int    `json:"max_hp"`
	Status  string `json:"status"`
	Attack  int    `json:"attack"`
	Defense int    `json:"defense"`
	Target  string `json:"target,omitempty"`
}

// ------------------------------------------------------------ finding NPCs

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

// ------------------------------------------------------------------- TALK

// talk returns the NPC's dialogue lines one after the other, in a loop.
func (s *State) talk(player *Player, typed string) Result {
	npc := s.findNPC(player, typed)
	if npc == nil {
		return reply(protocol.ErrNPCNotFound)
	}

	line := npc.Def.Dialogue[npc.TalkIndex%len(npc.Def.Dialogue)]
	npc.TalkIndex++

	logger.Info("npc_talk", logger.Fields{
		"player": player.Username, "npc": npc.Def.ID, "line": line,
	})

	return reply("OK " + line)
}

// ----------------------------------------------------------------- ATTACK

func (s *State) attack(player *Player, typed string) Result {
	npc := s.findNPC(player, typed)
	if npc == nil {
		return reply(protocol.ErrNPCNotFound)
	}

	if !npc.Def.Hostile {
		return reply(protocol.ErrNPCNotHostile)
	}

	if player.Target != npc.Def.ID {
		player.Target = npc.Def.ID
		logger.Info("combat_started", logger.Fields{
			"player": player.Username, "npc": npc.Def.ID, "room": player.Room,
		})
	}

	fightRoom := player.Room
	response := combatResponse{Attacker: player.Username, Target: npc.Def.ID}

	response.Damage = s.rollDamage(player.Attack, npc.Def.Defense)
	npc.HP -= response.Damage
	if npc.HP < 0 {
		npc.HP = 0
	}

	var events []Event

	if npc.HP == 0 {
		response.Status = StatusVictory
		s.defeatNPC(player, npc)
	} else {
		var defeated bool
		response.DamageTaken, defeated, events = s.enemyTurn(player, npc)
		response.Status = StatusCombat

		if defeated {
			response.Status = StatusDefeated
			response.Room = player.Room
		}
	}

	response.AttackerHP = player.HP
	response.TargetHP = npc.HP

	logger.Info("combat_round", logger.Fields{
		"player": player.Username, "npc": npc.Def.ID, "room": fightRoom,
		"damage": response.Damage, "damage_taken": response.DamageTaken,
		"player_hp": response.AttackerHP, "npc_hp": response.TargetHP,
		"status": response.Status,
	})

	line := okJSON(response)
	broadcast := s.toRoom(fightRoom, "EVT ROOM COMBAT "+line[len("OK "):], player.Username)

	return Result{Reply: line, Events: append([]Event{broadcast}, events...)}
}

func (s *State) rollDamage(attack int, defense int) int {
	damage := attack + s.random.Intn(5) - defense
	if damage < 1 {
		damage = 1
	}

	return damage
}

// enemyTurn: at 0 HP the player respawns and the presence events are returned.
func (s *State) enemyTurn(player *Player, npc *npcState) (int, bool, []Event) {
	damage := s.rollDamage(npc.Def.Attack, player.Defense)
	player.HP -= damage

	if player.HP > 0 {
		return damage, false, nil
	}

	logger.Info("player_defeated", logger.Fields{
		"player": player.Username, "npc": npc.Def.ID, "room": player.Room,
		"respawn_room": s.world.respawnRoom,
	})

	player.Target = ""
	player.HP = player.MaxHP / 2

	return damage, true, s.changeRoom(player, s.world.respawnRoom)
}

// defeatNPC credits the killer and their group members in the same room.
func (s *State) defeatNPC(killer *Player, npc *npcState) {
	npc.RespawnAt = time.Now().Add(EnemyRespawnDelay)

	logger.Info("npc_defeated", logger.Fields{
		"player": killer.Username, "npc": npc.Def.ID, "room": npc.Room,
	})

	for _, player := range s.players {
		if player.Target == npc.Def.ID {
			player.Target = ""
		}

		sameGroup := killer.Group != "" && player.Group == killer.Group
		if player == killer || (sameGroup && player.Room == killer.Room) {
			s.creditDefeat(player, npc.Def.ID)
		}
	}
}

// ----------------------------------------------------------------- STATUS

// currentEnemy returns the enemy the player is fighting, or nil.
func (s *State) currentEnemy(player *Player) *npcState {
	if player.Target == "" {
		return nil
	}

	npc := s.npcs[player.Target]
	if npc.Room != player.Room || !s.isAlive(npc) {
		player.Target = ""
		return nil
	}

	return npc
}

func (s *State) status(player *Player) Result {
	response := statusResponse{
		HP:      player.HP,
		MaxHP:   player.MaxHP,
		Attack:  player.Attack,
		Defense: player.Defense,
		Status:  "healthy",
	}

	if npc := s.currentEnemy(player); npc != nil {
		response.Status = StatusCombat
		response.Target = npc.Def.ID
	} else if player.HP < player.MaxHP {
		response.Status = "wounded"
	}

	return reply(okJSON(response))
}
