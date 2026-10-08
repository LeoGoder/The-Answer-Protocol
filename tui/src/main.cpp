#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cctype>
#include <csignal>
#include <cstring>
#include <filesystem>  // NOLINT(build/c++17)
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/node.hpp"

#include "../includes/connexion.hpp"
#include "../includes/image.hpp"
#include "../includes/parssing.hpp"

using ftxui::borderRounded;
using ftxui::CatchEvent;
using ftxui::Component;
using ftxui::Element;
using ftxui::Elements;
using ftxui::Event;
using ftxui::filler;
using ftxui::flex;
using ftxui::focus;
using ftxui::hbox;
using ftxui::Input;
using ftxui::Renderer;
using ftxui::ScreenInteractive;
using ftxui::text;
using ftxui::vbox;
using ftxui::window;
using ftxui::yframe;
using ftxui::color;
using ftxui::Color;
using ftxui::paragraph;

class ShrinkableNode : public ftxui::Node {
 public:
  explicit ShrinkableNode(ftxui::Element child)
      : ftxui::Node({std::move(child)}) {}

  void ComputeRequirement() override {
    ftxui::Node::ComputeRequirement();
    requirement_.min_x = 1;
    requirement_.flex_shrink_x = 1;
    requirement_.flex_grow_x = 1;
  }

  void SetBox(ftxui::Box box) override {
    ftxui::Node::SetBox(box);
    if (!children_.empty()) {
      children_[0]->SetBox(box);
    }
  }
};

static ftxui::Element shrinkable(ftxui::Element child) {
  return std::make_shared<ShrinkableNode>(std::move(child));
}

static ftxui::Element wrapLine(const std::string& pseudo,
                               const std::string& contenu,
                               ftxui::Color pseudo_color,
                               const std::string& channel = "") {
    if (pseudo.empty()) {
        return shrinkable(paragraph(contenu));
    }
    std::string prefix;
    if (!channel.empty()) {
        prefix = pseudo + " (" + channel + "): ";
    } else {
        prefix = pseudo + ": ";
    }
    return shrinkable(hbox({
        text(prefix) | color(pseudo_color),
        shrinkable(paragraph(contenu)) | flex,
    }));
}

struct ChatMessage {
    std::string pseudo;
    std::string contenu;
    std::string channel;
    bool is_local;
};

static std::vector<ChatMessage> messages;
static std::mutex messages_mutex;
static std::vector<ChatMessage> chat_messages;
static std::mutex chat_mutex;
static std::atomic<bool> interface_active{true};

enum PendingCommand {
    CMD_NONE,
    CMD_LOOK,
    CMD_QUEST,
    CMD_QUESTS,
    CMD_INVENTORY,
    CMD_TAKE,
    CMD_DROP,
    CMD_OTHER_JSON
};
static std::atomic<int> pending_command{CMD_NONE};

struct PendingChat {
    std::string pseudo;
    std::string contenu;
    std::string channel;
    std::string raw_command;
};

static std::queue<PendingChat> pending_chats;
static std::mutex pending_chat_mutex;
static std::mutex last_chat_mutex;
static std::string* user_pseudo = new std::string();
static std::string* last_chat_msg = new std::string();
static std::string* last_chat_channel = new std::string();
static std::string* current_map_image = new std::string(
    "assets/maps/default.png");
static std::mutex map_image_mutex;

static std::vector<QuestInfo> active_quests;
static std::vector<std::string> player_inventory;
static std::mutex quests_mutex;

static std::string roomToMapPath(const std::string& room_id,
                                 const std::string& room_name) {
    std::vector<std::string> candidates;

    if (!room_name.empty()) {
        std::string clean = room_name;
        for (char& c : clean) {
            if (c == ' ' || c == '-' || c == '.') c = '_';
            else c = static_cast<char>(std::tolower(c));
        }

        const std::string suffix = "_room";
        if (clean.size() > suffix.size() &&
            clean.substr(clean.size() - suffix.size()) == suffix) {
            std::string prefix = clean.substr(0, clean.size() - suffix.size());
            candidates.push_back("assets/maps/room_" + prefix + ".png");
        } else if (clean.rfind("room_", 0) == 0) {
            candidates.push_back("assets/maps/" + clean + ".png");
        } else {
            candidates.push_back("assets/maps/room_" + clean + ".png");
            candidates.push_back("assets/maps/" + clean + ".png");
        }
    }

    if (!room_id.empty()) {
        std::string id_clean = room_id;
        for (char& c : id_clean) {
            if (c == '.' || c == ' ' || c == '-') c = '_';
            else c = static_cast<char>(std::tolower(c));
        }
        candidates.push_back("assets/maps/" + id_clean + ".png");
    }

    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }

    if (!candidates.empty()) {
        return candidates[0];
    }

    return "assets/maps/default.png";
}

static void ajouterMessage(const std::string& pseudo,
                           const std::string& contenu,
                           bool is_local) {
    std::lock_guard<std::mutex> lock(messages_mutex);
    messages.push_back({pseudo, contenu, "", is_local});
}

static void ajouterChatMessage(const std::string& pseudo,
                               const std::string& contenu,
                               const std::string& channel,
                               bool is_local) {
    std::lock_guard<std::mutex> lock(chat_mutex);
    chat_messages.push_back({pseudo, contenu, channel, is_local});
}

struct ParsedChat {
    bool is_chat = false;
    std::string channel;
    std::string message;
    std::string command_to_send;
};

static ParsedChat parseChatCommand(const std::string& input) {
    ParsedChat result;
    size_t i = 0;
    while (i < input.size() &&
           std::isspace(static_cast<unsigned char>(input[i]))) {
        i++;
    }
    if (i >= input.size()) return result;

    size_t cmd_start = i;
    while (i < input.size() &&
           !std::isspace(static_cast<unsigned char>(input[i]))) {
        i++;
    }
    std::string cmd = input.substr(cmd_start, i - cmd_start);
    for (char& c : cmd) c = static_cast<char>(std::toupper(c));
    if (cmd != "CHAT") return result;

    while (i < input.size() &&
           std::isspace(static_cast<unsigned char>(input[i]))) {
        i++;
    }
    if (i >= input.size()) return result;

    size_t scope_start = i;
    while (i < input.size() &&
           !std::isspace(static_cast<unsigned char>(input[i]))) {
        i++;
    }
    std::string scope = input.substr(scope_start, i - scope_start);
    for (char& c : scope) c = static_cast<char>(std::toupper(c));

    std::string channel;
    std::string wire_scope;
    if (scope == "ROOM") {
        channel = "ROOM";
        wire_scope = "ROOM";
    } else if (scope == "GLOBAL") {
        channel = "GLOBAL";
        wire_scope = "GLOBAL";
    } else if (scope == "PARTY" || scope == "GROUP") {
        channel = "PARTY";
        wire_scope = "GROUP";
    } else {
        return result;
    }

    while (i < input.size() &&
           std::isspace(static_cast<unsigned char>(input[i]))) {
        i++;
    }
    if (i >= input.size()) return result;

    std::string msg = input.substr(i);
    while (!msg.empty() &&
           std::isspace(static_cast<unsigned char>(msg.back()))) {
        msg.pop_back();
    }
    if (msg.empty()) return result;

    result.is_chat = true;
    result.channel = channel;
    result.message = msg;
    result.command_to_send = "CHAT " + wire_scope + " " + msg;
    return result;
}

static void ecouterServeur(int socket_fd, ScreenInteractive& screen) {
    char buffer[1024];
    std::string reste;

    while (true) {
        ssize_t n = recv(socket_fd, buffer, sizeof(buffer), 0);
        if (n <= 0) break;

        reste.append(buffer, static_cast<size_t>(n));

        size_t pos;
        while ((pos = reste.find('\n')) != std::string::npos) {
            std::string ligne = reste.substr(0, pos);
            reste.erase(0, pos + 1);
            if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();

            int cmd = pending_command.load();

            // Handle OK responses with JSON payload based on pending command
            if (ligne.substr(0, 3) == "OK " && ligne.size() > 3) {
                if (cmd == CMD_LOOK) {
                    RoomInfo room;
                    if (parseLookResponse(ligne, room)) {
                        auto formatted = formatRoomInfo(room);
                        for (const auto& line : formatted) {
                            ajouterMessage("", line, false);
                        }
                        if (!room.name.empty() || !room.id.empty()) {
                            std::lock_guard<std::mutex> lock(map_image_mutex);
                            *current_map_image = roomToMapPath(
                                room.id, room.name);
                        }
                        pending_command = CMD_NONE;
                        continue;
                    }
                } else if (cmd == CMD_QUEST) {
                    QuestInfo quest;
                    if (parseQuestResponse(ligne, quest)) {
                        auto formatted = formatQuestInfo(quest);
                        for (const auto& line : formatted) {
                            ajouterMessage("", line, false);
                        }
                        {
                            std::lock_guard<std::mutex> lock(quests_mutex);
                            bool found = false;
                            for (auto& q : active_quests) {
                                if (q.quest_id == quest.quest_id) {
                                    q = quest;
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                active_quests.push_back(quest);
                            }
                        }
                        pending_command = CMD_NONE;
                        continue;
                    }
                } else if (cmd == CMD_QUESTS) {
                    std::vector<QuestInfo> quests;
                    if (parseQuestsResponse(ligne, quests)) {
                        {
                            std::lock_guard<std::mutex> lock(quests_mutex);
                            active_quests = quests;
                        }
                        if (quests.empty()) {
                            ajouterMessage("", "No active quests.", false);
                        } else {
                            for (const auto& q : quests) {
                                auto formatted = formatQuestInfo(q);
                                for (const auto& line : formatted) {
                                    ajouterMessage("", line, false);
                                }
                                ajouterMessage("", "---", false);
                            }
                        }
                        pending_command = CMD_NONE;
                        continue;
                    }
                } else if ((cmd == CMD_TAKE || cmd == CMD_DROP) &&
                           ligne.rfind("OK [", 0) == 0) {
                    std::vector<std::string> inventory;
                    if (parseInventoryResponse(ligne, inventory)) {
                        {
                            std::lock_guard<std::mutex> lock(quests_mutex);
                            player_inventory = inventory;
                        }
                        pending_command = CMD_NONE;
                        continue;
                    }
                }

                // For other commands with JSON, format generically
                if (cmd != CMD_NONE) {
                    std::vector<std::string> lines;
                    if (parseGenericOkJson(ligne, lines)) {
                        for (const auto& line : lines) {
                            ajouterMessage("", line, false);
                        }
                        pending_command = CMD_NONE;
                        continue;
                    }
                }
            }

            // Handle ERR when waiting for a command response
            if (cmd != CMD_NONE && ligne.rfind("ERR", 0) == 0) {
                pending_command = CMD_NONE;
            }

            if (ligne == "OK") {
                bool was_pending_chat = false;
                PendingChat pending;
                {
                    std::lock_guard<std::mutex> lock(pending_chat_mutex);
                    if (!pending_chats.empty()) {
                        pending = pending_chats.front();
                        pending_chats.pop();
                        was_pending_chat = true;
                    }
                }
                if (was_pending_chat) {
                    ajouterChatMessage(pending.pseudo, pending.contenu,
                                       pending.channel, true);
                    {
                        std::lock_guard<std::mutex> lock(last_chat_mutex);
                        *last_chat_msg = pending.contenu;
                        *last_chat_channel = pending.channel;
                    }
                    continue;
                }
                // Simple OK for non-chat command
                if (cmd != CMD_NONE) {
                    pending_command = CMD_NONE;
                }
            }
            if (ligne.rfind("ERR", 0) == 0) {
                bool was_pending_chat = false;
                PendingChat pending;
                {
                    std::lock_guard<std::mutex> lock(pending_chat_mutex);
                    if (!pending_chats.empty()) {
                        pending = pending_chats.front();
                        pending_chats.pop();
                        was_pending_chat = true;
                    }
                }
                if (was_pending_chat) {
                    ajouterMessage(pending.pseudo, pending.raw_command, true);
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
                    {
                        std::lock_guard<std::mutex> lock(last_chat_mutex);
                        if (pseudo == *user_pseudo && msg == *last_chat_msg &&
                            (last_chat_channel->empty() ||
                             channel == *last_chat_channel)) {
                            (*last_chat_msg).clear();
                            (*last_chat_channel).clear();
                            continue;
                        }
                    }
                    ajouterChatMessage(pseudo, msg, channel, false);
                } else {
                    ajouterChatMessage(evt_rest, "", channel, false);
                }
                continue;
            }

            ajouterMessage("Serveur", ligne, false);
        }
        screen.PostEvent(Event::Custom);
    }

    if (interface_active) {
        ajouterMessage("", "*** Connexion avec le serveur perdue ***", false);
        screen.PostEvent(Event::Custom);
    }
}

int main() {
    std::signal(SIGPIPE, SIG_IGN);

    InfosConnexion infos;
    std::string erreur_serveur;
    int sock = -1;

    while (true) {
        if (!ecranConnexion(infos, erreur_serveur)) {
            std::cout << "Connexion annulée.\n";
            return 0;
        }

        int port = -1;
        try {
            port = std::stoi(infos.port);
        } catch (...) {
        }
        if (port <= 0 || port > 65535) {
            erreur_serveur = "Port invalide (" + infos.port + ").";
            continue;
        }

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "Erreur : Impossible de créer le socket.\n";
            return 1;
        }

        sockaddr_in serv_addr;
        std::memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port   = htons(static_cast<uint16_t>(port));
        if (inet_pton(AF_INET, infos.ip.c_str(), &serv_addr.sin_addr) <= 0) {
            erreur_serveur = "Adresse IP invalide (" + infos.ip + ").";
            close(sock);
            sock = -1;
            continue;
        }

        if (connect(sock, reinterpret_cast<sockaddr*>(&serv_addr),
                    sizeof(serv_addr)) < 0) {
            erreur_serveur = "Connexion au serveur échouée.";
            close(sock);
            sock = -1;
            continue;
        }

        std::string cmd_connect = "CONNECT " + infos.pseudo + "\n";
        send(sock, cmd_connect.c_str(), cmd_connect.size(), 0);

        std::string reste;
        char buf[1024];
        int lignes_lues = 0;
        bool connexion_ok = false;

        while (lignes_lues < 2) {
            ssize_t n = recv(sock, buf, sizeof(buf), 0);
            if (n <= 0) {
                erreur_serveur = "Le serveur a fermé la connexion.";
                break;
            }
            reste.append(buf, static_cast<size_t>(n));

            size_t pos;
            while ((pos = reste.find(
                    '\n')) != std::string::npos && lignes_lues < 2) {
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
                    erreur_serveur = ligne;
                }
            }
        }

        if (connexion_ok) {
            break;
        }

        close(sock);
        sock = -1;
    }

    int port = std::stoi(infos.port);
    *user_pseudo = infos.pseudo;

    auto screen = ScreenInteractive::Fullscreen();

    std::string saisie;
    Component champ = Input(&saisie, "Tapez votre commande ici...");

    Component renderer = Renderer(champ, [&] {
        Elements msg_elements;
        {
            std::lock_guard<std::mutex> lock(messages_mutex);
            for (const auto& msg : messages) {
                if (msg.pseudo.empty()) {
                    msg_elements.push_back(wrapLine("", msg.contenu,
                        Color::White));
                } else if (msg.is_local) {
                    msg_elements.push_back(wrapLine(msg.pseudo, msg.contenu,
                        Color::Cyan));
                } else {
                    msg_elements.push_back(wrapLine(msg.pseudo, msg.contenu,
                        Color::Yellow));
                }
            }
        }
        if (!msg_elements.empty()) {
            msg_elements.back() = msg_elements.back() | focus;
        }

        std::string title = " Terminal - " + infos.pseudo + " @ " +
                            infos.ip + ":" + std::to_string(port) + " ";

        Elements item_elements;
        {
            std::lock_guard<std::mutex> lock(quests_mutex);
            if (player_inventory.empty()) {
                item_elements.push_back(
                    shrinkable(paragraph("  No items")) |
                        color(Color::GrayDark));
            } else {
                for (const auto& item : player_inventory) {
                    item_elements.push_back(
                        shrinkable(paragraph("  - " + item)) |
                            color(Color::White));
                }
            }
        }
        Element items_panel =
            window(text(" Items "),
                   vbox(item_elements) | yframe | flex) | flex;

        Elements quest_elements;
        {
            std::lock_guard<std::mutex> lock(quests_mutex);
            if (active_quests.empty()) {
                quest_elements.push_back(
                    shrinkable(paragraph("  No active quests")) |
                        color(Color::GrayDark));
            } else {
                for (const auto& q : active_quests) {
                    quest_elements.push_back(
                        shrinkable(paragraph("  " + q.description)) |
                            color(Color::Yellow));
                    std::string progress_str = "    (" + q.progress + ")";
                    if (!q.status.empty())
                        progress_str += " [" + q.status + "]";
                    quest_elements.push_back(
                        shrinkable(paragraph(progress_str)) |
                            color(Color::GrayLight));
                }
            }
        }
        Element quests_panel =
            window(text(" Quests "),
                   vbox(quest_elements) | yframe | flex) | flex;
        Element left_column =
            vbox({items_panel, quests_panel}) | flex;

        Element terminal_panel = vbox({
            window(text(title),
                   vbox(msg_elements) | yframe | flex) | flex,
            borderRounded(hbox({text(" > "),
                shrinkable(champ->Render()) | flex})),
        }) | flex;

        std::string map_path;
        {
            std::lock_guard<std::mutex> lock(map_image_mutex);
            map_path = *current_map_image;
        }
        Element map_panel =
            window(text(" Map "),
                   put_image(map_path, 0, 0, false) | flex) | flex;
        Elements chat_elements;
        {
            std::lock_guard<std::mutex> lock(chat_mutex);
            for (const auto& msg : chat_messages) {
                if (msg.is_local) {
                    chat_elements.push_back(wrapLine(msg.pseudo, msg.contenu,
                        Color::Cyan, msg.channel));
                } else {
                    chat_elements.push_back(wrapLine(msg.pseudo, msg.contenu,
                        Color::Yellow, msg.channel));
                }
            }
        }
        if (!chat_elements.empty()) {
            chat_elements.back() = chat_elements.back() | focus;
        }

        Element chat_panel =
            window(text(" Chat "),
                   vbox(chat_elements) | yframe | flex) | flex;
        Element right_column =
            vbox({map_panel, chat_panel}) | flex;

        return hbox({
            left_column,
            terminal_panel | flex,
            right_column,
        });
    });

    renderer |= CatchEvent([&](Event event) {
        if (event != Event::Return) return false;

        if (saisie == "QUIT") {
            screen.Exit();
            return true;
        }
        if (!saisie.empty()) {
            std::string cmd_upper = saisie;
            for (auto& c : cmd_upper) c = static_cast<char>(std::toupper(c));
            if (cmd_upper == "QUIT") {
                screen.Exit();
                return true;
            }
            if (cmd_upper == "LOOK") {
                pending_command = CMD_LOOK;
            } else if (cmd_upper.rfind("QUEST ", 0) == 0) {
                pending_command = CMD_QUEST;
            } else if (cmd_upper == "QUESTS") {
                pending_command = CMD_QUESTS;
            } else if (cmd_upper.rfind("TAKE ", 0) == 0) {
                pending_command = CMD_TAKE;
            } else if (cmd_upper.rfind("DROP ", 0) == 0) {
                pending_command = CMD_DROP;
            } else if (cmd_upper == "INVENTORY" ||
                       cmd_upper == "STATUS" ||
                       cmd_upper.rfind("TALK ", 0) == 0) {
                pending_command = CMD_OTHER_JSON;
            }
            ParsedChat chat = parseChatCommand(saisie);
            if (chat.is_chat) {
                {
                    std::lock_guard<std::mutex> lock(pending_chat_mutex);
                    pending_chats.push({infos.pseudo, chat.message,
                                        chat.channel, saisie});
                }
                std::string ligne = chat.command_to_send + "\n";
                send(sock, ligne.c_str(), ligne.size(), 0);
            } else {
                ajouterMessage(infos.pseudo, saisie, true);
                std::string ligne = saisie + "\n";
                send(sock, ligne.c_str(), ligne.size(), 0);
            }
            saisie.clear();
        }
        return true;
    });

    std::thread threadReception(ecouterServeur, sock, std::ref(screen));
    screen.Loop(renderer);

    interface_active = false;
    shutdown(sock, SHUT_RDWR);
    threadReception.join();
    close(sock);

    return 0;
}
