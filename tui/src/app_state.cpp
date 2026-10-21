#include "../includes/app_state.hpp"

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

void AppState::setActiveQuests(const std::vector<QuestInfo>& quests) {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    active_quests_ = quests;
}

void AppState::upsertQuest(const QuestInfo& quest) {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    for (auto& q : active_quests_) {
        if (q.quest_id == quest.quest_id) {
            q = quest;
            return;
        }
    }
    active_quests_.push_back(quest);
}

std::vector<QuestInfo> AppState::getActiveQuests() {
    std::lock_guard<std::mutex> lock(quests_mutex_);
    return active_quests_;
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
