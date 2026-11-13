/**
 * @file network_client.cpp
 * @brief TCP socket networking client and asynchronous response listener.
 *
 * Implements socket connection, message transmission, and background
 * line buffering / parsing for RFC-compliant TAP protocol exchanges.
 */

#include "../includes/network_client.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <ftxui/component/screen_interactive.hpp>

#include "../includes/map_resolver.hpp"
#include "../includes/protocol.hpp"
#include "../includes/ui_helpers.hpp"

/**
 * @brief Default constructor initializing socket file descriptor and atomic flags.
 */
NetworkClient::NetworkClient()
    : socket_fd_(-1), connection_lost_(false), user_quit_(false) {}

/**
 * @brief Destructor ensuring socket and background listener are cleanly disconnected.
 */
NetworkClient::~NetworkClient() {
    disconnect();
}

/**
 * @brief Connects to the remote TAP server and performs the initial handshake.
 *
 * Resolves IP, connects the TCP socket, and sends "CONNECT <pseudo>\n".
 * Expects the server greeting and confirmation response ("OK ...").
 *
 * @param config Connection configuration including host IP, port, and pseudo.
 * @param error_message Output string populated if connection fails.
 * @return True if connected and authenticated successfully, false otherwise.
 */
bool NetworkClient::connectToServer(const ConnectionConfig& config,
                                    std::string& error_message) {
    disconnect();
    connection_lost_.store(false);
    user_quit_.store(false);

    int port = -1;
    try {
        port = std::stoi(config.port);
    } catch (...) {
    }
    if (port <= 0 || port > 65535) {
        error_message = "Invalid port (" + config.port + ").";
        return false;
    }

    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        error_message = "Failed to create socket.";
        return false;
    }

    sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, config.ip.c_str(), &serv_addr.sin_addr) <= 0) {
        error_message = "Invalid IP address (" + config.ip + ").";
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    if (connect(socket_fd_, reinterpret_cast<sockaddr*>(&serv_addr),
                sizeof(serv_addr)) < 0) {
        error_message = "Failed to connect to server.";
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    // Send initial CONNECT handshake
    std::string cmd_connect = "CONNECT " + config.pseudo + "\n";
    send(socket_fd_, cmd_connect.c_str(), cmd_connect.size(), 0);

    // Read initial greeting and connection acknowledgment
    std::string stream_buffer;
    char buf[1024];
    int lines_read = 0;
    bool connection_ok = false;

    while (lines_read < 2) {
        ssize_t n = recv(socket_fd_, buf, sizeof(buf), 0);
        if (n <= 0) {
            error_message = "Server closed the connection.";
            break;
        }
        stream_buffer.append(buf, static_cast<size_t>(n));

        size_t pos;
        while ((pos = stream_buffer.find('\n')) != std::string::npos &&
               lines_read < 2) {
            std::string line = stream_buffer.substr(0, pos);
            stream_buffer.erase(0, pos + 1);
            if (!line.empty() && line.back() == '\r') line.pop_back();

            lines_read++;
            if (lines_read == 1) {
                // First line is server greeting banner
                continue;
            }
            if (line.substr(0, 2) == "OK") {
                connection_ok = true;
            } else {
                error_message = line;
            }
        }
    }

    if (!connection_ok) {
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    return true;
}

/**
 * @brief Disconnects the socket and terminates the background listener thread.
 */
void NetworkClient::disconnect() {
    if (socket_fd_ >= 0) {
        shutdown(socket_fd_, SHUT_RDWR);
    }
    stopListener();
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
}

/**
 * @brief Checks if the client has an open socket connection.
 * @return True if connected, false otherwise.
 */
bool NetworkClient::isConnected() const {
    return socket_fd_ >= 0;
}

/**
 * @brief Checks whether the connection was dropped unexpectedly by the remote host.
 * @return True if connection was lost, false otherwise.
 */
bool NetworkClient::isConnectionLost() const {
    return connection_lost_.load();
}

/**
 * @brief Sets whether the disconnection was initiated voluntarily by the user.
 * @param val True if user requested quit.
 */
void NetworkClient::setUserQuit(bool val) {
    user_quit_.store(val);
}

/**
 * @brief Sends a line of text terminated with a newline across the socket.
 *
 * Thread-safe via internal mutex.
 *
 * @param line String command to send.
 * @return True on success, false if sending failed or socket closed.
 */
bool NetworkClient::sendLine(const std::string& line) {
    std::lock_guard<std::mutex> lock(send_mutex_);
    if (socket_fd_ < 0) return false;
    std::string data = line + "\n";
    return send(socket_fd_, data.c_str(), data.size(), 0) >= 0;
}

/**
 * @brief Launches the asynchronous listener thread.
 *
 * @param screen Reference to active interactive FTXUI screen.
 * @param state Reference to shared application state.
 */
void NetworkClient::startListener(ftxui::ScreenInteractive& screen,
                                  AppState& state) {
    stopListener();
    listener_thread_ = std::make_unique<std::thread>(
        &NetworkClient::listenerLoop, this, std::ref(screen), std::ref(state));
}

/**
 * @brief Stops and joins the background listener thread.
 */
void NetworkClient::stopListener() {
    if (listener_thread_ && listener_thread_->joinable()) {
        listener_thread_->join();
    }
    listener_thread_.reset();
}

static void addLogLines(AppState& state, const std::vector<std::string>& lines) {
    for (const auto& l : lines) {
        state.addMessage("", l, false);
    }
}

// Updates UI room description and resolves map asset
void NetworkClient::updateRoomState(const RoomInfo& room, AppState& state) {
    addLogLines(state, formatRoomInfo(room));
    if (!room.id.empty() || !room.name.empty()) {
        state.setMapImage(resolveMapPath(room.id, room.name));
    }
}

// Handles server responses to CMD_MOVE (parsing room JSON or triggering LOOK fallback)
bool NetworkClient::handleMoveResponse(const std::string& line,
                                       AppState& state) {
    if (!startsWith(line, "OK")) return false;
    state.addMessage("Server", line, false);
    if (startsWith(line, "OK ") && line.find('{') != std::string::npos) {
        RoomInfo room;
        if (parseLookResponse(line, room)) {
            updateRoomState(room, state);
            state.setPendingCommand(CMD_NONE);
            return true;
        }
    }
    // Server returned bare OK on move: trigger automatic LOOK to update room description & map
    state.setPendingCommand(CMD_LOOK);
    sendLine("LOOK");
    return true;
}

// Handles server responses to TAKE, DROP, and INVENTORY queries
bool NetworkClient::handleInventoryResponse(const std::string& line, int cmd,
                                            AppState& state) {
    if (cmd == CMD_TAKE || cmd == CMD_DROP) {
        if (!startsWith(line, "OK")) return false;
        state.addMessage("Server", line, false);
        if (startsWith(line, "OK [")) {
            std::vector<std::string> inventory;
            if (parseInventoryResponse(line, inventory)) {
                state.setInventory(inventory);
                state.setPendingCommand(CMD_NONE);
                return true;
            }
        }
        state.setPendingCommand(CMD_INVENTORY_AUTO);
        sendLine("INVENTORY");
        return true;
    }

    if (cmd == CMD_INVENTORY || cmd == CMD_INVENTORY_AUTO) {
        if (!startsWith(line, "OK [")) return false;
        std::vector<std::string> inventory;
        if (parseInventoryResponse(line, inventory)) {
            state.setInventory(inventory);
            if (cmd == CMD_INVENTORY) {
                if (inventory.empty()) {
                    state.addMessage("", "Inventory is empty.", false);
                } else {
                    std::string inv_str = "Inventory: ";
                    for (size_t i = 0; i < inventory.size(); ++i) {
                        if (i > 0) inv_str += ", ";
                        inv_str += inventory[i];
                    }
                    state.addMessage("", inv_str, false);
                }
            }
            state.setPendingCommand(CMD_NONE);
            return true;
        }
    }
    return false;
}

// Handles item consumption responses and triggers automatic inventory update
bool NetworkClient::handleUseResponse(const std::string& line,
                                      AppState& state) {
    if (!startsWith(line, "OK")) return false;
    UseInfo use_info;
    if (parseUseResponse(line, use_info)) {
        addLogLines(state, formatUseInfo(use_info));
    } else {
        state.addMessage("Server", line, false);
    }
    state.setPendingCommand(CMD_INVENTORY_AUTO);
    sendLine("INVENTORY");
    return true;
}

// Handles attack replies, formats combat results, and triggers LOOK on victory
bool NetworkClient::handleAttackResponse(const std::string& line,
                                         AppState& state) {
    if (!startsWith(line, "OK")) return false;
    CombatInfo combat_info;
    if (parseCombatResponse(line, combat_info)) {
        addLogLines(state, formatCombatInfo(combat_info));
        if (combat_info.status == "defeated") {
            state.setPendingCommand(CMD_LOOK);
            sendLine("LOOK");
            return true;
        }
    } else {
        state.addMessage("Server", line, false);
    }
    state.setPendingCommand(CMD_NONE);
    return true;
}

// Deserializes and formats structured JSON responses for LOOK, QUEST, and QUESTS
bool NetworkClient::handleJsonQueryResponse(const std::string& line, int cmd,
                                            AppState& state) {
    if (!startsWith(line, "OK ") || line.size() <= 3) return false;

    if (cmd == CMD_LOOK) {
        RoomInfo room;
        if (parseLookResponse(line, room)) {
            updateRoomState(room, state);
            state.setPendingCommand(CMD_NONE);
            return true;
        }
    } else if (cmd == CMD_QUEST) {
        QuestInfo quest;
        if (parseQuestResponse(line, quest)) {
            addLogLines(state, formatQuestInfo(quest));
            state.upsertQuest(quest);
            state.setPendingCommand(CMD_NONE);
            return true;
        }
    } else if (cmd == CMD_QUESTS) {
        std::vector<QuestInfo> quests;
        if (parseQuestsResponse(line, quests)) {
            state.setActiveQuests(quests);
            if (quests.empty()) {
                state.addMessage("", "No active quests.", false);
            } else {
                for (auto q : quests) {
                    if (q.description.empty()) {
                        std::string desc = state.getQuestDescription(q.quest_id);
                        if (!desc.empty()) q.description = desc;
                    }
                    addLogLines(state, formatQuestInfo(q));
                    state.addMessage("", "---", false);
                }
            }
            state.setPendingCommand(CMD_NONE);
            return true;
        }
    } else if (cmd != CMD_NONE) {
        std::vector<std::string> lines;
        if (parseGenericOkJson(line, lines)) {
            addLogLines(state, lines);
            state.setPendingCommand(CMD_NONE);
            return true;
        }
    }

    return false;
}

// Handles bare OK / ERR acknowledgments for optimistic in-flight chat messages
bool NetworkClient::handleChatAck(const std::string& line, int cmd,
                                  AppState& state) {
    if (line != "OK") return false;
    PendingChat pending;
    if (state.popPendingChat(pending)) {
        state.addChatMessage(pending.pseudo, pending.content,
                             pending.channel, true);
        state.recordLastChat(pending.content, pending.channel);
        return true;
    }
    if (cmd != CMD_NONE) {
        state.setPendingCommand(CMD_NONE);
    }
    return false;
}

// Parses asynchronous server broadcast events (combat notices and multi-channel chat)
bool NetworkClient::handleAsyncEvent(const std::string& line,
                                     AppState& state) {
    if (!startsWith(line, "EVT ")) return false;
    std::string after_evt = line.substr(4);

    if (startsWith(after_evt, "ROOM COMBAT ")) {
        CombatInfo combat_info;
        if (parseCombatResponse(line, combat_info)) {
            std::string att = combat_info.attacker.empty()
                                  ? "Someone"
                                  : combat_info.attacker;
            std::string tgt = combat_info.target;
            if (!tgt.empty()) {
                tgt[0] = static_cast<char>(std::toupper(tgt[0]));
            }
            std::string msg = "[Combat] " + att + " attacked " + tgt + " (" +
                              std::to_string(combat_info.damage) + " dmg, " +
                              tgt + " HP: " +
                              std::to_string(combat_info.target_hp) + ")";
            state.addMessage("", msg, false);
            return true;
        }
    }

    static const std::pair<const char*, const char*> channels[] = {
        {"ROOM CHAT ", "ROOM"},
        {"GLOBAL CHAT ", "GLOBAL"},
        {"PARTY CHAT ", "PARTY"},
        {"GROUP CHAT ", "PARTY"}
    };

    std::string channel, evt_rest;
    for (const auto& [prefix, chan] : channels) {
        if (startsWith(after_evt, prefix)) {
            channel = chan;
            evt_rest = after_evt.substr(std::strlen(prefix));
            break;
        }
    }

    if (!channel.empty()) {
        size_t space = evt_rest.find(' ');
        if (space != std::string::npos) {
            std::string pseudo = evt_rest.substr(0, space);
            std::string msg = evt_rest.substr(space + 1);
            if (state.checkAndClearIfLastChat(pseudo, msg, channel)) {
                return true;
            }
            state.addChatMessage(pseudo, msg, channel, false);
        } else {
            state.addChatMessage(evt_rest, "", channel, false);
        }
        return true;
    }

    return false;
}

// Dispatches a single framed protocol line across specialized handlers
void NetworkClient::processIncomingLine(const std::string& line,
                                        AppState& state,
                                        ftxui::ScreenInteractive& screen) {
    if (startsWith(line, "ERR")) {
        state.setPendingCommand(CMD_NONE);
        PendingChat pending;
        if (state.popPendingChat(pending)) {
            state.addMessage(pending.pseudo, pending.raw_command, true);
        }
        state.addMessage("Server", line, false);
        return;
    }

    // Graceful quit confirmation (RFC 5.1.4)
    if (line == "OK bye" || startsWith(line, "OK bye")) {
        state.setPendingCommand(CMD_NONE);
        state.addMessage("Server", line, false);
        user_quit_.store(true);
        screen.Exit();
        return;
    }

    int cmd = state.getPendingCommand();

    if (cmd == CMD_MOVE && handleMoveResponse(line, state)) return;
    if ((cmd == CMD_TAKE || cmd == CMD_DROP || cmd == CMD_INVENTORY ||
         cmd == CMD_INVENTORY_AUTO) &&
        handleInventoryResponse(line, cmd, state)) return;
    if (cmd == CMD_USE && handleUseResponse(line, state)) return;
    if (cmd == CMD_ATTACK && handleAttackResponse(line, state)) return;
    if (cmd != CMD_NONE && handleJsonQueryResponse(line, cmd, state)) return;
    if (handleChatAck(line, cmd, state)) return;
    if (handleAsyncEvent(line, state)) return;

    // Fallback: unhandled server messages sent to main log
    state.addMessage("Server", line, false);
}

/**
 * @brief Background loop continuously receiving TCP packets and framing newline-terminated lines.
 *
 * Slices socket byte stream into lines, delegates protocol dispatching to processIncomingLine(),
 * triggers FTXUI screen redraws, and detects server disconnection.
 *
 * @param screen FTXUI screen reference to receive refresh events.
 * @param state Central application state.
 */
void NetworkClient::listenerLoop(ftxui::ScreenInteractive& screen,
                                 AppState& state) {
    char buffer[1024];
    std::string stream_buffer;

    while (true) {
        ssize_t n = recv(socket_fd_, buffer, sizeof(buffer), 0);
        if (n <= 0) break;

        stream_buffer.append(buffer, static_cast<size_t>(n));

        size_t pos;
        while ((pos = stream_buffer.find('\n')) != std::string::npos) {
            std::string line = stream_buffer.substr(0, pos);
            stream_buffer.erase(0, pos + 1);
            if (!line.empty() && line.back() == '\r') line.pop_back();

            processIncomingLine(line, state, screen);
        }
        screen.PostEvent(ftxui::Event::Custom);
    }

    if (state.isInterfaceActive()) {
        if (!user_quit_.load()) {
            connection_lost_.store(true);
        }
        screen.Exit();
    }
}
