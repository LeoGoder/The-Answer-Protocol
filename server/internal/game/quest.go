package game

import (
	"tap/server/internal/logger"
	"tap/server/internal/protocol"
)

// QUEST <npc>: finish or show the current quest, else give the next one, else 406.

type questInfo struct {
	QuestID     string `json:"quest_id"`
	Description string `json:"description"`
	Reward      string `json:"reward"`
	Status      string `json:"status"`
	Progress    string `json:"progress"`
}

type questLogEntry struct {
	QuestID  string `json:"quest_id"`
	Status   string `json:"status"`
	Progress string `json:"progress,omitempty"`
}

func (s *State) quest(player *Player, typed string) string {
	npc := s.findNPC(player, typed)
	if npc == nil {
		return protocol.ErrNPCNotFound
	}

	for _, progress := range player.Quests {
		quest := s.world.quests[progress.QuestID]
		if progress.Completed || quest.Giver != npc.Def.ID {
			continue
		}

		if s.objectiveDone(player, quest, progress) {
			return s.completeQuest(player, quest, progress)
		}

		return s.questReply(player, quest, progress, "active")
	}

	for _, id := range s.world.questOrder {
		quest := s.world.quests[id]
		if quest.Giver != npc.Def.ID || findProgress(player, id) != nil {
			continue
		}

		progress := &questProgress{QuestID: id}
		player.Quests = append(player.Quests, progress)

		logger.Info("quest_accepted", logger.Fields{
			"player": player.Username, "quest": id, "npc": npc.Def.ID,
		})

		return s.questReply(player, quest, progress, "active")
	}

	return protocol.ErrNoQuestAvailable
}

func (s *State) completeQuest(player *Player, quest *QuestDef, progress *questProgress) string {
	// The fetched item goes back to where it spawns.
	if quest.Type == QuestFetch {
		s.consumeItem(player, quest.Target)
	}

	progress.Completed = true

	player.MaxHP += quest.Reward.MaxHP
	player.HP += quest.Reward.MaxHP
	player.Attack += quest.Reward.Attack

	logger.Info("quest_completed", logger.Fields{
		"player": player.Username, "quest": quest.ID, "reward": quest.Reward.Name,
	})

	return s.questReply(player, quest, progress, "completed")
}

func (s *State) questReply(player *Player, quest *QuestDef, progress *questProgress, status string) string {
	return okJSON(questInfo{
		QuestID:     quest.ID,
		Description: quest.Description,
		Reward:      quest.Reward.Name,
		Status:      status,
		Progress:    s.progressText(player, quest, progress),
	})
}

func (s *State) questLog(player *Player) string {
	entries := make([]questLogEntry, 0, len(player.Quests))

	for _, progress := range player.Quests {
		quest := s.world.quests[progress.QuestID]

		if progress.Completed {
			entries = append(entries, questLogEntry{QuestID: quest.ID, Status: "completed"})
			continue
		}

		entries = append(entries, questLogEntry{
			QuestID:  quest.ID,
			Status:   "active",
			Progress: s.progressText(player, quest, progress),
		})
	}

	return okJSON(entries)
}

// creditDefeat is called by combat when an enemy is defeated.
func (s *State) creditDefeat(player *Player, npcID string) {
	for _, progress := range player.Quests {
		quest := s.world.quests[progress.QuestID]

		if progress.Completed || progress.Defeated {
			continue
		}

		if quest.Type == QuestDefeat && quest.Target == npcID {
			progress.Defeated = true

			logger.Info("quest_progress", logger.Fields{
				"player": player.Username, "quest": quest.ID, "progress": "1/1",
			})
		}
	}
}

func (s *State) objectiveDone(player *Player, quest *QuestDef, progress *questProgress) bool {
	if quest.Type == QuestFetch {
		_, found := indexOf(player.Inventory, quest.Target)
		return found
	}

	return progress.Defeated
}

func (s *State) progressText(player *Player, quest *QuestDef, progress *questProgress) string {
	if progress.Completed || s.objectiveDone(player, quest, progress) {
		return "1/1"
	}

	return "0/1"
}

func findProgress(player *Player, questID string) *questProgress {
	for _, progress := range player.Quests {
		if progress.QuestID == questID {
			return progress
		}
	}

	return nil
}

func indexOf(list []string, value string) (int, bool) {
	for index, current := range list {
		if current == value {
			return index, true
		}
	}

	return -1, false
}
