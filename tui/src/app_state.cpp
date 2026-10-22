#include "../includes/app_state.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

AppState::AppState()
    : current_map_image_("assets/maps/default.png"),
      pending_command_(CMD_NONE),
      interface_active_(true) {}

void AppState::addMessage(const std::string& pseudo,
                          const std::string& content,
                          bool is_local) {
    std::lock_guard<std::mutex> lock(messages_mutex_);
    messages_.push_back({pseudo, content, "", is_local});
}

std::vector<ChatMessage> AppState::getMessages() {
    std::lock_guard<std::mutex> lock(messages_mutex_);
    return messages_;
}

void AppState::clearMessages() {
    std::lock_guard<std::mutex> lock(messages_mutex_);
    messages_.clear();
}

void AppState::addChatMessage(const std::string& pseudo,
                              const std::string& content,
                              const std::string& channel,
                              bool is_local) {
    std::lock_guard<std::mutex> lock(chat_mutex_);
    chat_messages_.push_back({pseudo, content, channel, is_local});
}

std::vector<ChatMessage> AppState::getChatMessages() {
    std::lock_guard<std::mutex> lock(chat_mutex_);
    return chat_messages_;
}

static bool isQuestCompleted(const QuestInfo& q) {
    std::string s = q.status;
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return (s == "completed" || s == "complete" || s == "done" ||
            s == "finished" || s == "success");
}

static void mergeQuestInfo(QuestInfo& target, const QuestInfo& source) {
    if (target.quest_id.empty()) target.quest_id = source.quest_id;
    if (target.description.empty()) target.description = source.description;
    if (target.reward.empty()) target.reward = source.reward;
    if (target.objective.empty()) target.objective = source.objective;
    if (target.target_item.empty()) target.target_item = source.target_item;
    if (target.target_npc.empty()) target.target_npc = source.target_npc;
    if (target.required_items == 0) target.required_items = source.required_items;
    if (target.progress.empty()) target.progress = source.progress;
    if (target.status.empty()) target.status = source.status;
}

void AppState::setActiveQuests(const std::vector<QuestInfo>& quests) {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    std::vector<QuestInfo> merged;
    merged.reserve(quests.size());

    for (auto q : quests) {
        auto it_cache = quest_cache_.find(q.quest_id);
        if (it_cache != quest_cache_.end()) {
            mergeQuestInfo(q, it_cache->second);
        }

        auto it_active = std::find_if(active_quests_.begin(), active_quests_.end(),
                                      [&](const QuestInfo& existing) {
                                          return existing.quest_id == q.quest_id;
                                      });
        if (it_active != active_quests_.end()) {
            mergeQuestInfo(q, *it_active);
        }

        quest_cache_[q.quest_id] = q;

        if (dismissed_quest_ids_.count(q.quest_id) && isQuestCompleted(q)) {
            continue;
        }

        merged.push_back(q);
    }

    active_quests_ = std::move(merged);
}

void AppState::upsertQuest(const QuestInfo& quest) {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    QuestInfo q = quest;

    dismissed_quest_ids_.erase(q.quest_id);

    auto it_cache = quest_cache_.find(q.quest_id);
    if (it_cache != quest_cache_.end()) {
        mergeQuestInfo(q, it_cache->second);
    }

    for (auto& existing : active_quests_) {
        if (existing.quest_id == q.quest_id) {
            mergeQuestInfo(q, existing);
            existing = q;
            quest_cache_[q.quest_id] = q;
            return;
        }
    }

    quest_cache_[q.quest_id] = q;
    active_quests_.push_back(q);
}

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

std::vector<QuestInfo> AppState::getActiveQuests() {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    return active_quests_;
}

std::string AppState::getQuestDescription(const std::string& quest_id) {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    auto it = quest_cache_.find(quest_id);
    if (it != quest_cache_.end()) {
        return it->second.description;
    }
    return "";
}

void AppState::setInventory(const std::vector<std::string>& inventory) {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    player_inventory_ = inventory;
}

std::vector<std::string> AppState::getInventory() {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    return player_inventory_;
}

void AppState::setMapImage(const std::string& path) {
    std::lock_guard<std::mutex> lock(map_image_mutex_);
    current_map_image_ = path;
}

std::string AppState::getMapImage() {
    std::lock_guard<std::mutex> lock(map_image_mutex_);
    return current_map_image_;
}

void AppState::setUserPseudo(const std::string& pseudo) {
    std::lock_guard<std::mutex> lock(user_pseudo_mutex_);
    user_pseudo_ = pseudo;
}

std::string AppState::getUserPseudo() {
    std::lock_guard<std::mutex> lock(user_pseudo_mutex_);
    return user_pseudo_;
}

void AppState::setPendingCommand(int cmd) {
    pending_command_.store(cmd);
}

int AppState::getPendingCommand() const {
    return pending_command_.load();
}

void AppState::pushPendingChat(const PendingChat& chat) {
    std::lock_guard<std::mutex> lock(pending_chat_mutex_);
    pending_chats_.push(chat);
}

bool AppState::popPendingChat(PendingChat& out) {
    std::lock_guard<std::mutex> lock(pending_chat_mutex_);
    if (pending_chats_.empty()) {
        return false;
    }
    out = pending_chats_.front();
    pending_chats_.pop();
    return true;
}

void AppState::recordLastChat(const std::string& msg,
                              const std::string& channel) {
    std::lock_guard<std::mutex> lock(last_chat_mutex_);
    last_chat_msg_ = msg;
    last_chat_channel_ = channel;
}

bool AppState::checkAndClearIfLastChat(const std::string& pseudo,
                                      const std::string& msg,
                                      const std::string& channel) {
    std::string current_user = getUserPseudo();
    std::lock_guard<std::mutex> lock(last_chat_mutex_);
    if (pseudo == current_user && msg == last_chat_msg_ &&
        (last_chat_channel_.empty() || channel == last_chat_channel_)) {
        last_chat_msg_.clear();
        last_chat_channel_.clear();
        return true;
    }
    return false;
}

bool AppState::isInterfaceActive() const {
    return interface_active_.load();
}

void AppState::setInterfaceActive(bool active) {
    interface_active_.store(active);
}

void AppState::addCommandHistory(const std::string& cmd) {
    if (cmd.empty()) return;
    std::lock_guard<std::mutex> lock(command_history_mutex_);
    if (!command_history_.empty() && command_history_.back() == cmd) {
        return;
    }
    command_history_.push_back(cmd);
    const size_t MAX_HISTORY = 100;
    if (command_history_.size() > MAX_HISTORY) {
        command_history_.erase(command_history_.begin());
    }
}

std::vector<std::string> AppState::getCommandHistory() {
    std::lock_guard<std::mutex> lock(command_history_mutex_);
    return command_history_;
}
