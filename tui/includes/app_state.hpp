/**
 * @file app_state.hpp
 * @brief Thread-safe centralized state manager for the TAP TUI client.
 */

#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "../includes/models.hpp"

/**
 * @class AppState
 * @brief Centralized thread-safe storage connecting network listener and UI loop.
 *
 * AppState orchestrates synchronized access to terminal logs, chat messages,
 * inventory items, active quests, room map paths, and command input history
 * across concurrent threads using mutexes and atomic primitives.
 */
class AppState {
 public:
    // Initializes default state, map path, and interface activity flag
    AppState();
    ~AppState() = default;

    // Terminal log messages
    void addMessage(const std::string& pseudo,
                    const std::string& content,
                    bool is_local = false);
    std::vector<ChatMessage> getMessages();
    void clearMessages();

    // Multi-channel chat messages
    void addChatMessage(const std::string& pseudo,
                        const std::string& content,
                        const std::string& channel,
                        bool is_local = false);
    std::vector<ChatMessage> getChatMessages();

    /*
     * Updates active quest list while preserving cached descriptions and progress.
     * Merges incoming quest records with quest_cache_ and filters out dismissed quests.
     */
    void setActiveQuests(const std::vector<QuestInfo>& quests);

    // Inserts or updates an individual quest in active list and persistent cache
    void upsertQuest(const QuestInfo& quest);

    // Removes completed quests from display and marks them as dismissed
    void clearCompletedQuests();

    // Retrieves snapshot of currently active quests
    std::vector<QuestInfo> getActiveQuests();

    // Looks up cached quest description by quest identifier
    std::string getQuestDescription(const std::string& quest_id);

    // Updates player inventory items list
    void setInventory(const std::vector<std::string>& inventory);

    // Retrieves snapshot of player inventory items
    std::vector<std::string> getInventory();

    // Sets current room map image file path
    void setMapImage(const std::string& path);

    // Retrieves current room map image file path
    std::string getMapImage();

    // Sets logged-in user pseudo
    void setUserPseudo(const std::string& pseudo);

    // Retrieves logged-in user pseudo
    std::string getUserPseudo();

    // Sets pending protocol command identifier
    void setPendingCommand(int cmd);

    // Retrieves pending protocol command identifier
    int getPendingCommand() const;

    // Enqueues locally sent chat message awaiting server ACK
    void pushPendingChat(const PendingChat& chat);

    // Dequeues oldest pending chat message upon server OK ACK
    bool popPendingChat(PendingChat& out);

    // Records last sent chat message to detect server echo reflection
    void recordLastChat(const std::string& msg, const std::string& channel);

    /*
     * Checks if incoming chat event matches the locally recorded sent chat.
     * Suppresses duplicate echo messages reflected back by the server.
     */
    bool checkAndClearIfLastChat(const std::string& pseudo,
                                 const std::string& msg,
                                 const std::string& channel);

    // Checks whether interactive UI event loop is currently active
    bool isInterfaceActive() const;

    // Sets active status flag of the interactive UI loop
    void setInterfaceActive(bool active);

    /*
     * Records command string into navigation history.
     * Enforces duplicate suppression against immediate previous command and caps size at 100.
     */
    void addCommandHistory(const std::string& cmd);

    // Retrieves full command navigation history list
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
