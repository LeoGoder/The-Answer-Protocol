#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "../includes/models.hpp"

class AppState {
 public:
    AppState();
    ~AppState() = default;

    // Terminal messages
    void addMessage(const std::string& pseudo,
                    const std::string& content,
                    bool is_local = false);
    std::vector<ChatMessage> getMessages();
    void clearMessages();

    // Chat messages
    void addChatMessage(const std::string& pseudo,
                        const std::string& content,
                        const std::string& channel,
                        bool is_local = false);
    std::vector<ChatMessage> getChatMessages();

    // Quests & Inventory
    void setActiveQuests(const std::vector<QuestInfo>& quests);
    void upsertQuest(const QuestInfo& quest);
    void clearCompletedQuests();
    std::vector<QuestInfo> getActiveQuests();
    std::string getQuestDescription(const std::string& quest_id);

    void setInventory(const std::vector<std::string>& inventory);
    std::vector<std::string> getInventory();

    // Map image
    void setMapImage(const std::string& path);
    std::string getMapImage();

    // User pseudo
    void setUserPseudo(const std::string& pseudo);
    std::string getUserPseudo();

    // Pending command
    void setPendingCommand(int cmd);
    int getPendingCommand() const;

    // Pending chats queue
    void pushPendingChat(const PendingChat& chat);
    bool popPendingChat(PendingChat& out);

    // Chat deduplication
    void recordLastChat(const std::string& msg, const std::string& channel);
    bool checkAndClearIfLastChat(const std::string& pseudo,
                                 const std::string& msg,
                                 const std::string& channel);

    // Interface active flag
    bool isInterfaceActive() const;
    void setInterfaceActive(bool active);

    // Command history
    void addCommandHistory(const std::string& cmd);
    std::vector<std::string> getCommandHistory();

 private:
    std::vector<ChatMessage> messages_;
    std::mutex messages_mutex_;

    std::vector<ChatMessage> chat_messages_;
    std::mutex chat_mutex_;

    std::vector<QuestInfo> active_quests_;
    std::map<std::string, QuestInfo> quest_cache_;
    std::set<std::string> dismissed_quest_ids_;
    std::vector<std::string> player_inventory_;
    std::mutex quests_mutex_;

    std::string current_map_image_;
    std::mutex map_image_mutex_;

    std::string user_pseudo_;
    std::mutex user_pseudo_mutex_;

    std::atomic<int> pending_command_{CMD_NONE};

    std::queue<PendingChat> pending_chats_;
    std::mutex pending_chat_mutex_;

    std::string last_chat_msg_;
    std::string last_chat_channel_;
    std::mutex last_chat_mutex_;

    std::atomic<bool> interface_active_{true};

    std::vector<std::string> command_history_;
    std::mutex command_history_mutex_;
};
