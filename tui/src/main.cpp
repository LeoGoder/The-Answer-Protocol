#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <csignal>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
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
                               ftxui::Color pseudo_color) {
    if (pseudo.empty()) {
        return shrinkable(paragraph(contenu));
    }
    return shrinkable(hbox({
        text(pseudo + ": ") | color(pseudo_color),
        shrinkable(paragraph(contenu)) | flex,
    }));
}

struct ChatMessage {
    std::string pseudo;
    std::string contenu;
    bool is_local;
};

static std::vector<ChatMessage> messages;
static std::mutex messages_mutex;
static std::vector<ChatMessage> chat_messages;
static std::mutex chat_mutex;
static std::atomic<bool> interface_active{true};
static std::atomic<bool> attente_look{false};
static std::atomic<bool> attente_chat_ok{false};
static std::string user_pseudo;
static std::string last_chat_msg;
static std::mutex last_chat_mutex;

static void ajouterMessage(const std::string& pseudo,
                           const std::string& contenu,
                           bool is_local) {
    std::lock_guard<std::mutex> lock(messages_mutex);
    messages.push_back({pseudo, contenu, is_local});
}

static void ajouterChatMessage(const std::string& pseudo,
                               const std::string& contenu,
                               bool is_local) {
    std::lock_guard<std::mutex> lock(chat_mutex);
    chat_messages.push_back({pseudo, contenu, is_local});
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

            // Si on attend une réponse LOOK, tenter de la parser
            if (attente_look.load() && ligne.substr(0, 3) == "OK ") {
                RoomInfo room;
                if (parseLookResponse(ligne, room)) {
                    auto formatted = formatRoomInfo(room);
                    for (const auto& line : formatted) {
                        ajouterMessage("", line, false);
                    }
                    attente_look = false;
                    continue;
                }
            }
            if (attente_look.load() && ligne.substr(0, 3) == "ERR") {
                attente_look = false;
            }
            // Supprimer le OK du serveur après une commande CHAT
            if (attente_chat_ok.load() && ligne == "OK") {
                attente_chat_ok = false;
                continue;
            }
            // Intercepter les messages EVT ROOM CHAT pseudo message
            const std::string evt_prefix = "EVT ROOM CHAT ";
            if (ligne.size() > evt_prefix.size() &&
                ligne.substr(0, evt_prefix.size()) == evt_prefix) {
                std::string rest = ligne.substr(evt_prefix.size());
                // Le premier mot est le pseudo, le reste est le message
                size_t space = rest.find(' ');
                if (space != std::string::npos) {
                    std::string pseudo = rest.substr(0, space);
                    std::string msg = rest.substr(space + 1);
                    // Si c'est notre propre message renvoyé par le serveur, on l'ignore
                    {
                        std::lock_guard<std::mutex> lock(last_chat_mutex);
                        if (pseudo == user_pseudo && msg == last_chat_msg) {
                            last_chat_msg.clear();
                            continue;
                        }
                    }
                    ajouterChatMessage(pseudo, msg, false);
                } else {
                    ajouterChatMessage(rest, "", false);
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
    if (!ecranConnexion(infos)) {
        std::cout << "Connexion annulée.\n";
        return 0;
    }

    int port = -1;
    try {
        port = std::stoi(infos.port);
    } catch (...) {
    }
    if (port <= 0 || port > 65535) {
        std::cerr << "Erreur : port invalide (" << infos.port << ").\n";
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Erreur : Impossible de créer le socket.\n";
        return 1;
    }

    sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, infos.ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Erreur : adresse IP invalide (" << infos.ip << ").\n";
        close(sock);
        return 1;
    }

    std::cout << "Connexion à " << infos.ip << ":" << port << "...\n";
    if (connect(sock, reinterpret_cast<sockaddr*>(&serv_addr),
                sizeof(serv_addr)) < 0) {
        std::cerr << "Erreur : Connexion au serveur échouée.\n";
        close(sock);
        return 1;
    }

    std::string cmd_connect = "CONNECT " + infos.pseudo + "\n";
    std::cout << "[ENVOI SERVEUR] " << cmd_connect.substr(
        0, cmd_connect.size() - 1)
              << " (vers " << infos.ip << ":" << port << ")\n";
    send(sock, cmd_connect.c_str(), cmd_connect.size(), 0);
    user_pseudo = infos.pseudo;

    auto screen = ScreenInteractive::Fullscreen();

    std::string saisie;
    Component champ = Input(&saisie, "Tapez votre commande ici...");

    Component renderer = Renderer(champ, [&] {
        Elements msg_elements;
        {
            std::lock_guard<std::mutex> lock(messages_mutex);
            for (const auto& msg : messages) {
                if (msg.pseudo.empty()) {
                    msg_elements.push_back(wrapLine("", msg.contenu, Color::White));
                } else if (msg.is_local) {
                    msg_elements.push_back(wrapLine(msg.pseudo, msg.contenu, Color::Cyan));
                } else {
                    msg_elements.push_back(wrapLine(msg.pseudo, msg.contenu, Color::Yellow));
                }
            }
        }
        if (!msg_elements.empty()) {
            msg_elements.back() = msg_elements.back() | focus;
        }

        std::string title = " Terminal - " + infos.pseudo + " @ " +
                            infos.ip + ":" + std::to_string(port) + " ";

        Element items_panel =
            window(text(" Items "), filler()) | flex;
        Element quests_panel =
            window(text(" Quests "), filler()) | flex;
        Element left_column =
            vbox({items_panel, quests_panel}) | flex;

        Element terminal_panel = vbox({
            window(text(title),
                   vbox(msg_elements) | yframe | flex) | flex,
            borderRounded(hbox({text(" > "), shrinkable(champ->Render()) | flex})),
        }) | flex;

        Element map_panel =
            window(text(" Map "),
                   put_image("temp_image.png", 0, 0, false) | flex) | flex;
        // Construire les éléments du chat
        Elements chat_elements;
        {
            std::lock_guard<std::mutex> lock(chat_mutex);
            for (const auto& msg : chat_messages) {
                if (msg.is_local) {
                    chat_elements.push_back(wrapLine(msg.pseudo, msg.contenu, Color::Cyan));
                } else {
                    chat_elements.push_back(wrapLine(msg.pseudo, msg.contenu, Color::Yellow));
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
            // Détecter si c'est une commande LOOK
            std::string cmd_upper = saisie;
            for (auto& c : cmd_upper) c = static_cast<char>(std::toupper(c));
            if (cmd_upper == "LOOK") {
                attente_look = true;
            }
            // Détecter si c'est une commande CHAT (ex: "chat room bonjour")
            bool is_chat = false;
            if (cmd_upper.substr(0, 4) == "CHAT") {
                // Extraire le message après "chat room " ou "chat "
                std::string chat_msg;
                if (cmd_upper.size() > 10 && cmd_upper.substr(0, 10) == "CHAT ROOM ") {
                    chat_msg = saisie.substr(10);
                } else if (saisie.size() > 5) {
                    chat_msg = saisie.substr(5);
                }
                if (!chat_msg.empty()) {
                    ajouterChatMessage(infos.pseudo, chat_msg, true);
                    // Stocker pour filtrer l'écho du serveur
                    {
                        std::lock_guard<std::mutex> lock(last_chat_mutex);
                        last_chat_msg = chat_msg;
                    }
                    attente_chat_ok = true;
                    is_chat = true;
                }
            }
            // Ne pas afficher les commandes CHAT dans le terminal
            if (!is_chat) {
                ajouterMessage(infos.pseudo, saisie, true);
            }
            std::string ligne = saisie + "\n";
            send(sock, ligne.c_str(), ligne.size(), 0);
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
