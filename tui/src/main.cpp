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

#include "../includes/connexion.hpp"
#include "../includes/image.hpp"

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

static std::vector<std::string> messages;
static std::mutex messages_mutex;
static std::atomic<bool> interface_active{true};

static void ajouterMessage(const std::string& msg) {
    std::lock_guard<std::mutex> lock(messages_mutex);
    messages.push_back(msg);
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
            ajouterMessage("Serveur: " + ligne);
        }
        screen.PostEvent(Event::Custom);
    }

    if (interface_active) {
        ajouterMessage("*** Connexion avec le serveur perdue ***");
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

    auto screen = ScreenInteractive::Fullscreen();

    std::string saisie;
    Component champ = Input(&saisie, "Tapez votre commande ici...");

    Component renderer = Renderer(champ, [&] {
        Elements msg_elements;
        {
            std::lock_guard<std::mutex> lock(messages_mutex);
            for (const auto& msg : messages) {
                msg_elements.push_back(text(msg));
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
            borderRounded(hbox({text(" > "), champ->Render()})),
        }) | flex;

        Element map_panel =
            window(text(" Map "),
                   put_image("temp_image.png", 0, 0, false) | flex) | flex;
        Element chat_panel =
            window(text(" Chat "), filler()) | flex;
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
            ajouterMessage("Moi: " + saisie);
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
