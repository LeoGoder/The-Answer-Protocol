// Thread-safe application state

#include "../includes/app_state.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>
#include <vector>

// Constructor initializing default map path and active interface flag
AppState::AppState()
    : current_map_image_("assets/maps/default.png"),
      pending_command_(CMD_NONE),
      interface_active_(true) {}

// Thread-safe append to terminal message log
void AppState::addMessage(const std::string& pseudo,
                          const std::string& content,
                          bool is_local) {
    std::lock_guard<std::mutex> lock(messages_mutex_);
    messages_.push_back({pseudo, content, "", is_local});
}

// Thread-safe copy of terminal messages
std::vector<ChatMessage> AppState::getMessages() {
    std::lock_guard<std::mutex> lock(messages_mutex_);
    return messages_;
}

// Thread-safe clear of terminal log
void AppState::clearMessages() {
    std::lock_guard<std::mutex> lock(messages_mutex_);
    messages_.clear();
}

// Append message to designated chat channel
void AppState::addChatMessage(const std::string& pseudo,
                              const std::string& content,
                              const std::string& channel,
                              bool is_local) {
    std::lock_guard<std::mutex> lock(chat_mutex_);
    chat_messages_.push_back({pseudo, content, channel, is_local});
}

// Thread-safe copy of all chat messages
std::vector<ChatMessage> AppState::getChatMessages() {
    std::lock_guard<std::mutex> lock(chat_mutex_);
    return chat_messages_;
}

// Checks if quest status matches completion synonyms
static bool isQuestCompleted(const QuestInfo& q) {
    std::string s = q.status;
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return (s == "completed" || s == "complete" || s == "done" ||
            s == "finished" || s == "success");
}

// Merges non-empty fields from source quest into target quest
static void mergeQuestInfo(QuestInfo& target, const QuestInfo& source) {
    if (target.quest_id.empty()) target.quest_id = source.quest_id;
    if (target.description.empty()) target.description = source.description;
    if (target.reward.empty()) target.reward = source.reward;
    if (target.objective.empty()) target.objective = source.objective;
    if (target.target_item.empty()) target.target_item = source.target_item;
    if (target.target_npc.empty()) target.target_npc = source.target_npc;
    if (target.required_items == 0) {
        target.required_items = source.required_items;
    }
    if (target.progress.empty()) target.progress = source.progress;
    if (target.status.empty()) target.status = source.status;
}

/*
 * Updates active quest list while preserving cached descriptions and progress from earlier queries.
 * Merges new data with quest_cache_ and filters out dismissed completed quests.
 */
void AppState::setActiveQuests(const std::vector<QuestInfo>& quests) {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    std::vector<QuestInfo> merged;
    merged.reserve(quests.size());

    for (auto q : quests) {
        // Recover historical description and details previously cached
        auto it_cache = quest_cache_.find(q.quest_id);
        if (it_cache != quest_cache_.end()) {
            mergeQuestInfo(q, it_cache->second);
        }

        // Retain progress tracking from previous active state
        auto it_active = std::find_if(
            active_quests_.begin(), active_quests_.end(),
            [&](const QuestInfo& existing) {
                return existing.quest_id == q.quest_id;
            });
        if (it_active != active_quests_.end()) {
            mergeQuestInfo(q, *it_active);
        }

        quest_cache_[q.quest_id] = q;

        // Skip quests previously dismissed by user after completion
        if (dismissed_quest_ids_.count(q.quest_id) && isQuestCompleted(q)) {
            continue;
        }

        merged.push_back(q);
    }

    active_quests_ = std::move(merged);
}

// Inserts or updates an individual quest in active list and persistent cache
void AppState::upsertQuest(const QuestInfo& quest) {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    QuestInfo q = quest;

    // Reactivate quest if it was previously dismissed
    dismissed_quest_ids_.erase(q.quest_id);

    // Merge cached fields (e.g. detailed objective and description)
    auto it_cache = quest_cache_.find(q.quest_id);
    if (it_cache != quest_cache_.end()) {
        mergeQuestInfo(q, it_cache->second);
    }

    // Update existing entry in active quests list if already present
    for (auto& existing : active_quests_) {
        if (existing.quest_id == q.quest_id) {
            mergeQuestInfo(q, existing);
            existing = q;
            quest_cache_[q.quest_id] = q;
            return;
        }
    }

    // New quest: append to active list
    quest_cache_[q.quest_id] = q;
    active_quests_.push_back(q);
}

// Removes completed quests from display and marks them as dismissed
void AppState::clearCompletedQuests() {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    std::vector<QuestInfo> remaining;
    for (const auto& q : active_quests_) {
        if (isQuestCompleted(q)) {
            dismissed_quest_ids_.insert(q.quest_id);
        } else {
            remaining.push_back(q);
        }
    }
    active_quests_ = std::move(remaining);
}

// Retrieves list of active quests
std::vector<QuestInfo> AppState::getActiveQuests() {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    return active_quests_;
}

// Looks up cached quest description by ID
std::string AppState::getQuestDescription(const std::string& quest_id) {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    auto it = quest_cache_.find(quest_id);
    if (it != quest_cache_.end()) {
        return it->second.description;
    }
    return "";
}

// Updates player inventory items
void AppState::setInventory(const std::vector<std::string>& inventory) {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    player_inventory_ = inventory;
}

// Retrieves player inventory items
std::vector<std::string> AppState::getInventory() {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    return player_inventory_;
}

// Sets room map image path
void AppState::setMapImage(const std::string& path) {
    std::lock_guard<std::mutex> lock(map_image_mutex_);
    current_map_image_ = path;
}

// Retrieves current room map image path
std::string AppState::getMapImage() {
    std::lock_guard<std::mutex> lock(map_image_mutex_);
    return current_map_image_;
}

// Sets user pseudo
void AppState::setUserPseudo(const std::string& pseudo) {
    std::lock_guard<std::mutex> lock(user_pseudo_mutex_);
    user_pseudo_ = pseudo;
}

// Retrieves user pseudo
std::string AppState::getUserPseudo() {
    std::lock_guard<std::mutex> lock(user_pseudo_mutex_);
    return user_pseudo_;
}

// Sets pending protocol command ID
void AppState::setPendingCommand(int cmd) {
    pending_command_.store(cmd);
}

// Retrieves pending protocol command ID
int AppState::getPendingCommand() const {
    return pending_command_.load();
}

// Enqueues locally sent chat message awaiting server ACK
void AppState::pushPendingChat(const PendingChat& chat) {
    std::lock_guard<std::mutex> lock(pending_chat_mutex_);
    pending_chats_.push(chat);
}

// Dequeues oldest pending chat message upon server OK ACK
bool AppState::popPendingChat(PendingChat& out) {
    std::lock_guard<std::mutex> lock(pending_chat_mutex_);
    if (pending_chats_.empty()) {
        return false;
    }
    out = pending_chats_.front();
    pending_chats_.pop();
    return true;
}

// Records last sent chat message to detect and suppress server echo loops
void AppState::recordLastChat(const std::string& msg,
                              const std::string& channel) {
    std::lock_guard<std::mutex> lock(last_chat_mutex_);
    last_chat_msg_ = msg;
    last_chat_channel_ = channel;
}

/*
 * Suppresses duplicate chat echoes: stores locally sent chat so that incoming EVT broadcasts
 * reflected back by the server to the sender are recognized and discarded.
 */
bool AppState::checkAndClearIfLastChat(const std::string& pseudo,
                                      const std::string& msg,
                                      const std::string& channel) {
    std::string current_user = getUserPseudo();
    std::lock_guard<std::mutex> lock(last_chat_mutex_);
    // Match sender pseudo, message body, and channel against recorded outgoing message
    if (pseudo == current_user && msg == last_chat_msg_ &&
        (last_chat_channel_.empty() || channel == last_chat_channel_)) {
        // Clear echo buffer upon match
        last_chat_msg_.clear();
        last_chat_channel_.clear();
        return true;
    }
    return false;
}

// Checks whether UI interface loop is active
bool AppState::isInterfaceActive() const {
    return interface_active_.load();
}

// Sets active status of UI interface
void AppState::setInterfaceActive(bool active) {
    interface_active_.store(active);
}

// Adds command to navigation history with duplicate suppression and 100-entry limit
void AppState::addCommandHistory(const std::string& cmd) {
    if (cmd.empty()) return;
    std::lock_guard<std::mutex> lock(command_history_mutex_);
    // Suppress immediate repeated commands (like pressing Return multiple times)
    if (!command_history_.empty() && command_history_.back() == cmd) {
        return;
    }
    command_history_.push_back(cmd);
    // Cap navigation history buffer to 100 entries to bound memory usage
    const size_t MAX_HISTORY = 100;
    if (command_history_.size() > MAX_HISTORY) {
        command_history_.erase(command_history_.begin());
    }
}

// Retrieves full command navigation history
std::vector<std::string> AppState::getCommandHistory() {
    std::lock_guard<std::mutex> lock(command_history_mutex_);
    return command_history_;
}
