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

/**
 * @brief Core worker loop for background socket receiving and protocol dispatch.
 *
 * Continuously reads from socket, frames incoming data into newline-separated
 * messages, matches them with pending commands or server events (EVT), parses
 * JSON payloads, updates AppState, and requests FTXUI screen redraws.
 *
 * @param screen FTXUI screen reference to trigger redraw events.
 * @param state Shared application state.
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

            int cmd = state.getPendingCommand();

            // Handling response to CMD_MOVE
            if (cmd == CMD_MOVE) {
                if (line.rfind("ERR", 0) == 0) {
                    state.setPendingCommand(CMD_NONE);
                    state.addMessage("Server", line, false);
                    continue;
                }
                if (line.rfind("OK", 0) == 0) {
                    state.addMessage("Server", line, false);
                    if (line.substr(0, 3) == "OK " &&
                        line.find('{') != std::string::npos) {
                        RoomInfo room;
                        if (parseLookResponse(line, room)) {
                            auto formatted = formatRoomInfo(room);
                            for (const auto& item_line : formatted) {
                                state.addMessage("", item_line, false);
                            }
                            if (!room.id.empty() || !room.name.empty()) {
                                state.setMapImage(
                                    resolveMapPath(room.id, room.name));
                            }
                            state.setPendingCommand(CMD_NONE);
                            continue;
                        }
                    }
                    // Server returned bare OK on move: trigger automatic LOOK to update room description & map
                    state.setPendingCommand(CMD_LOOK);
                    sendLine("LOOK");
                    continue;
                }
            }

            // Handling response to CMD_TAKE / CMD_DROP
            if (cmd == CMD_TAKE || cmd == CMD_DROP) {
                if (line.rfind("ERR", 0) == 0) {
                    state.setPendingCommand(CMD_NONE);
                    state.addMessage("Server", line, false);
                    continue;
                }
                if (line.rfind("OK", 0) == 0) {
                    state.addMessage("Server", line, false);
                    if (line.rfind("OK [", 0) == 0) {
                        std::vector<std::string> inventory;
                        if (parseInventoryResponse(line, inventory)) {
                            state.setInventory(inventory);
                            state.setPendingCommand(CMD_NONE);
                            continue;
                        }
                    }
                    // Server returned bare OK: trigger automatic INVENTORY query to refresh items panel
                    state.setPendingCommand(CMD_INVENTORY_AUTO);
                    sendLine("INVENTORY");
                    continue;
                }
            }

            // Handling response to CMD_USE
            if (cmd == CMD_USE) {
                if (line.rfind("ERR", 0) == 0) {
                    state.setPendingCommand(CMD_NONE);
                    state.addMessage("Server", line, false);
                    continue;
                }
                if (line.rfind("OK", 0) == 0) {
                    UseInfo use_info;
                    if (parseUseResponse(line, use_info)) {
                        auto formatted = formatUseInfo(use_info);
                        for (const auto& item_line : formatted) {
                            state.addMessage("", item_line, false);
                        }
                    } else {
                        state.addMessage("Server", line, false);
                    }
                    // Item consumed: refresh inventory quantities automatically
                    state.setPendingCommand(CMD_INVENTORY_AUTO);
                    sendLine("INVENTORY");
                    continue;
                }
            }

            // Handling response to CMD_ATTACK
            if (cmd == CMD_ATTACK) {
                if (line.rfind("ERR", 0) == 0) {
                    state.setPendingCommand(CMD_NONE);
                    state.addMessage("Server", line, false);
                    continue;
                }
                if (line.rfind("OK", 0) == 0) {
                    CombatInfo combat_info;
                    if (parseCombatResponse(line, combat_info)) {
                        auto formatted = formatCombatInfo(combat_info);
                        for (const auto& item_line : formatted) {
                            state.addMessage("", item_line, false);
                        }
                        if (combat_info.status == "defeated") {
                            // Target eliminated: trigger automatic LOOK to reflect dropped loot and room changes
                            state.setPendingCommand(CMD_LOOK);
                            sendLine("LOOK");
                            continue;
                        }
                    } else {
                        state.addMessage("Server", line, false);
                    }
                    state.setPendingCommand(CMD_NONE);
                    continue;
                }
            }

            // Handling response to CMD_INVENTORY / CMD_INVENTORY_AUTO
            if (cmd == CMD_INVENTORY || cmd == CMD_INVENTORY_AUTO) {
                if (line.rfind("ERR", 0) == 0) {
                    state.setPendingCommand(CMD_NONE);
                    state.addMessage("Server", line, false);
                    continue;
                }
                if (line.rfind("OK [", 0) == 0) {
                    std::vector<std::string> inventory;
                    if (parseInventoryResponse(line, inventory)) {
                        state.setInventory(inventory);
                        if (cmd == CMD_INVENTORY) {
                            if (inventory.empty()) {
                                state.addMessage(
                                    "", "Inventory is empty.", false);
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
                        continue;
                    }
                }
            }

            // Handling generic OK responses with JSON payload
            if (line.substr(0, 3) == "OK " && line.size() > 3) {
                if (cmd == CMD_LOOK) {
                    RoomInfo room;
                    if (parseLookResponse(line, room)) {
                        auto formatted = formatRoomInfo(room);
                        for (const auto& item_line : formatted) {
                            state.addMessage("", item_line, false);
                        }
                        if (!room.id.empty() || !room.name.empty()) {
                            state.setMapImage(
                                resolveMapPath(room.id, room.name));
                        }
                        state.setPendingCommand(CMD_NONE);
                        continue;
                    }
                } else if (cmd == CMD_QUEST) {
                    QuestInfo quest;
                    if (parseQuestResponse(line, quest)) {
                        auto formatted = formatQuestInfo(quest);
                        for (const auto& item_line : formatted) {
                            state.addMessage("", item_line, false);
                        }
                        state.upsertQuest(quest);
                        state.setPendingCommand(CMD_NONE);
                        continue;
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
                                    std::string desc =
                                        state.getQuestDescription(q.quest_id);
                                    if (!desc.empty()) q.description = desc;
                                }
                                auto formatted = formatQuestInfo(q);
                                for (const auto& item_line : formatted) {
                                    state.addMessage("", item_line, false);
                                }
                                state.addMessage("", "---", false);
                            }
                        }
                        state.setPendingCommand(CMD_NONE);
                        continue;
                    }
                }

                if (cmd != CMD_NONE) {
                    std::vector<std::string> lines;
                    if (parseGenericOkJson(line, lines)) {
                        for (const auto& item_line : lines) {
                            state.addMessage("", item_line, false);
                        }
                        state.setPendingCommand(CMD_NONE);
                        continue;
                    }
                }
            }

            if (cmd != CMD_NONE && line.rfind("ERR", 0) == 0) {
                state.setPendingCommand(CMD_NONE);
            }

            // Bare OK response: could be chat acknowledgment
            if (line == "OK") {
                PendingChat pending;
                // Pop optimistic chat: confirmation that server accepted our outgoing chat message
                if (state.popPendingChat(pending)) {
                    state.addChatMessage(pending.pseudo, pending.content,
                                         pending.channel, true);
                    state.recordLastChat(pending.content, pending.channel);
                    continue;
                }
                if (cmd != CMD_NONE) {
                    state.setPendingCommand(CMD_NONE);
                }
            }
            if (line.rfind("ERR", 0) == 0) {
                PendingChat pending;
                // Chat rejected by server: print error into main terminal log
                if (state.popPendingChat(pending)) {
                    state.addMessage(pending.pseudo, pending.raw_command, true);
                }
            }

            // Asynchronous event parsing (EVT)
            std::string channel;
            std::string evt_rest;
            if (line.substr(0, 4) == "EVT ") {
                std::string after_evt = line.substr(4);
                // Broadcast room combat events between any entities
                if (after_evt.substr(0, 12) == "ROOM COMBAT ") {
                    CombatInfo combat_info;
                    if (parseCombatResponse(line, combat_info)) {
                        std::string att = combat_info.attacker.empty()
                                              ? "Someone"
                                              : combat_info.attacker;
                        std::string tgt = combat_info.target;
                        if (!tgt.empty()) {
                            tgt[0] = static_cast<char>(std::toupper(tgt[0]));
                        }
                        std::string dmg_str =
                            std::to_string(combat_info.damage);
                        std::string hp_str =
                            std::to_string(combat_info.target_hp);
                        std::string msg = "[Combat] " + att + " attacked " +
                                          tgt + " (" + dmg_str + " dmg, " +
                                          tgt + " HP: " + hp_str + ")";
                        state.addMessage("", msg, false);
                        continue;
                    }
                // Room and global chat broadcast channels
                } else if (after_evt.substr(0, 10) == "ROOM CHAT ") {
                    channel = "ROOM";
                    evt_rest = after_evt.substr(10);
                } else if (after_evt.substr(0, 12) == "GLOBAL CHAT ") {
                    channel = "GLOBAL";
                    evt_rest = after_evt.substr(12);
                } else if (after_evt.substr(0, 11) == "PARTY CHAT ") {
                    channel = "PARTY";
                    evt_rest = after_evt.substr(11);
                } else if (after_evt.substr(0, 11) == "GROUP CHAT ") {
                    channel = "PARTY";
                    evt_rest = after_evt.substr(11);
                }
            }
            if (!channel.empty()) {
                size_t space = evt_rest.find(' ');
                if (space != std::string::npos) {
                    std::string pseudo = evt_rest.substr(0, space);
                    std::string msg = evt_rest.substr(space + 1);
                    // Discard incoming echo if it matches our own message just acknowledged
                    if (state.checkAndClearIfLastChat(pseudo, msg, channel)) {
                        continue;
                    }
                    state.addChatMessage(pseudo, msg, channel, false);
                } else {
                    state.addChatMessage(evt_rest, "", channel, false);
                }
                continue;
            }

            // Fallback: unhandled server messages sent to main log
            state.addMessage("Server", line, false);
        }
        screen.PostEvent(ftxui::Event::Custom);
    }

    if (state.isInterfaceActive() && !user_quit_.load()) {
        connection_lost_.store(true);
        screen.Exit();
    }
}
