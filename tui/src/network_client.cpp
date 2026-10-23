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

NetworkClient::NetworkClient() : socket_fd_(-1) {}

NetworkClient::~NetworkClient() {
    disconnect();
}

bool NetworkClient::connectToServer(const ConnectionConfig& config,
                                    std::string& error_message) {
    int port = -1;
    try {
        port = std::stoi(config.port);
    } catch (...) {
    }
    if (port <= 0 || port > 65535) {
        error_message = "Port invalide (" + config.port + ").";
        return false;
    }

    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        error_message = "Impossible de créer le socket.";
        return false;
    }

    sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, config.ip.c_str(), &serv_addr.sin_addr) <= 0) {
        error_message = "Adresse IP invalide (" + config.ip + ").";
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    if (connect(socket_fd_, reinterpret_cast<sockaddr*>(&serv_addr),
                sizeof(serv_addr)) < 0) {
        error_message = "Connexion au serveur échouée.";
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    std::string cmd_connect = "CONNECT " + config.pseudo + "\n";
    send(socket_fd_, cmd_connect.c_str(), cmd_connect.size(), 0);

    std::string reste;
    char buf[1024];
    int lignes_lues = 0;
    bool connexion_ok = false;

    while (lignes_lues < 2) {
        ssize_t n = recv(socket_fd_, buf, sizeof(buf), 0);
        if (n <= 0) {
            error_message = "Le serveur a fermé la connexion.";
            break;
        }
        reste.append(buf, static_cast<size_t>(n));

        size_t pos;
        while ((pos = reste.find('\n')) != std::string::npos &&
               lignes_lues < 2) {
            std::string ligne = reste.substr(0, pos);
            reste.erase(0, pos + 1);
            if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();

            lignes_lues++;
            if (lignes_lues == 1) {
                continue;
            }
            if (ligne.substr(0, 2) == "OK") {
                connexion_ok = true;
            } else {
                error_message = ligne;
            }
        }
    }

    if (!connexion_ok) {
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    return true;
}

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

bool NetworkClient::isConnected() const {
    return socket_fd_ >= 0;
}

bool NetworkClient::sendLine(const std::string& line) {
    std::lock_guard<std::mutex> lock(send_mutex_);
    if (socket_fd_ < 0) return false;
    std::string data = line + "\n";
    return send(socket_fd_, data.c_str(), data.size(), 0) >= 0;
}

void NetworkClient::startListener(ftxui::ScreenInteractive& screen,
                                  AppState& state) {
    stopListener();
    listener_thread_ = std::make_unique<std::thread>(
        &NetworkClient::listenerLoop, this, std::ref(screen), std::ref(state));
}

void NetworkClient::stopListener() {
    if (listener_thread_ && listener_thread_->joinable()) {
        listener_thread_->join();
    }
    listener_thread_.reset();
}

void NetworkClient::listenerLoop(ftxui::ScreenInteractive& screen,
                                 AppState& state) {
    char buffer[1024];
    std::string reste;

    while (true) {
        ssize_t n = recv(socket_fd_, buffer, sizeof(buffer), 0);
        if (n <= 0) break;

        reste.append(buffer, static_cast<size_t>(n));

        size_t pos;
        while ((pos = reste.find('\n')) != std::string::npos) {
            std::string ligne = reste.substr(0, pos);
            reste.erase(0, pos + 1);
            if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();

            int cmd = state.getPendingCommand();

            if (cmd == CMD_MOVE) {
                if (ligne.rfind("ERR", 0) == 0) {
                    state.setPendingCommand(CMD_NONE);
                    state.addMessage("Serveur", ligne, false);
                    continue;
                }
                if (ligne.rfind("OK", 0) == 0) {
                    state.addMessage("Serveur", ligne, false);
                    if (ligne.substr(0, 3) == "OK " &&
                        ligne.find('{') != std::string::npos) {
                        RoomInfo room;
                        if (parseLookResponse(ligne, room)) {
                            auto formatted = formatRoomInfo(room);
                            for (const auto& line : formatted) {
                                state.addMessage("", line, false);
                            }
                            if (!room.id.empty() || !room.name.empty()) {
                                state.setMapImage(
                                    resolveMapPath(room.id, room.name));
                            }
                            state.setPendingCommand(CMD_NONE);
                            continue;
                        }
                    }
                    state.setPendingCommand(CMD_LOOK);
                    sendLine("LOOK");
                    continue;
                }
            }

            if (cmd == CMD_TAKE || cmd == CMD_DROP) {
                if (ligne.rfind("ERR", 0) == 0) {
                    state.setPendingCommand(CMD_NONE);
                    state.addMessage("Serveur", ligne, false);
                    continue;
                }
                if (ligne.rfind("OK", 0) == 0) {
                    state.addMessage("Serveur", ligne, false);
                    if (ligne.rfind("OK [", 0) == 0) {
                        std::vector<std::string> inventory;
                        if (parseInventoryResponse(ligne, inventory)) {
                            state.setInventory(inventory);
                            state.setPendingCommand(CMD_NONE);
                            continue;
                        }
                    }
                    state.setPendingCommand(CMD_INVENTORY_AUTO);
                    sendLine("INVENTORY");
                    continue;
                }
            }

            if (cmd == CMD_INVENTORY || cmd == CMD_INVENTORY_AUTO) {
                if (ligne.rfind("ERR", 0) == 0) {
                    state.setPendingCommand(CMD_NONE);
                    state.addMessage("Serveur", ligne, false);
                    continue;
                }
                if (ligne.rfind("OK [", 0) == 0) {
                    std::vector<std::string> inventory;
                    if (parseInventoryResponse(ligne, inventory)) {
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

            if (ligne.substr(0, 3) == "OK " && ligne.size() > 3) {
                if (cmd == CMD_LOOK) {
                    RoomInfo room;
                    if (parseLookResponse(ligne, room)) {
                        auto formatted = formatRoomInfo(room);
                        for (const auto& line : formatted) {
                            state.addMessage("", line, false);
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
                    if (parseQuestResponse(ligne, quest)) {
                        auto formatted = formatQuestInfo(quest);
                        for (const auto& line : formatted) {
                            state.addMessage("", line, false);
                        }
                        state.upsertQuest(quest);
                        state.setPendingCommand(CMD_NONE);
                        continue;
                    }
                } else if (cmd == CMD_QUESTS) {
                    std::vector<QuestInfo> quests;
                    if (parseQuestsResponse(ligne, quests)) {
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
                                for (const auto& line : formatted) {
                                    state.addMessage("", line, false);
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
                    if (parseGenericOkJson(ligne, lines)) {
                        for (const auto& line : lines) {
                            state.addMessage("", line, false);
                        }
                        state.setPendingCommand(CMD_NONE);
                        continue;
                    }
                }
            }

            if (cmd != CMD_NONE && ligne.rfind("ERR", 0) == 0) {
                state.setPendingCommand(CMD_NONE);
            }

            if (ligne == "OK") {
                PendingChat pending;
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
            if (ligne.rfind("ERR", 0) == 0) {
                PendingChat pending;
                if (state.popPendingChat(pending)) {
                    state.addMessage(pending.pseudo, pending.raw_command, true);
                }
            }
            std::string channel;
            std::string evt_rest;
            if (ligne.substr(0, 4) == "EVT ") {
                std::string after_evt = ligne.substr(4);
                if (after_evt.substr(0, 10) == "ROOM CHAT ") {
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
                    if (state.checkAndClearIfLastChat(pseudo, msg, channel)) {
                        continue;
                    }
                    state.addChatMessage(pseudo, msg, channel, false);
                } else {
                    state.addChatMessage(evt_rest, "", channel, false);
                }
                continue;
            }

            state.addMessage("Serveur", ligne, false);
        }
        screen.PostEvent(ftxui::Event::Custom);
    }

    if (state.isInterfaceActive()) {
        state.addMessage("", "*** Connexion avec le serveur perdue ***", false);
        screen.PostEvent(ftxui::Event::Custom);
    }
}
