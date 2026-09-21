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

using namespace ftxui;

// --- État partagé entre le thread réseau et l'interface ---
static std::vector<std::string> messages;
static std::mutex messages_mutex;                // protège "messages"
static std::atomic<bool> interface_active{true}; // false une fois l'UI fermée

static void ajouterMessage(const std::string& msg) {
    std::lock_guard<std::mutex> lock(messages_mutex);
    messages.push_back(msg);
}

// Thread de réception : lit le serveur et demande à FTXUI de se redessiner
static void ecouterServeur(int socket_fd, ScreenInteractive& screen) {
    char buffer[1024];
    std::string reste; // données reçues mais pas encore terminées par '\n'

    while (true) {
        ssize_t n = recv(socket_fd, buffer, sizeof(buffer), 0);
        if (n <= 0) break;

        reste.append(buffer, static_cast<size_t>(n));

        // Le protocole est basé sur des lignes : on découpe sur '\n'
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
    // Évite de tuer le programme si on écrit sur un socket fermé
    std::signal(SIGPIPE, SIG_IGN);

    // 1. Écran de connexion (connexion.cpp)
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

    // 2. Création du socket TCP
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Erreur : Impossible de créer le socket.\n";
        return 1;
    }

    // 3. Adresse IP et port saisis par l'utilisateur
    sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, infos.ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Erreur : adresse IP invalide (" << infos.ip << ").\n";
        close(sock);
        return 1;
    }

    // 4. Connexion au serveur
    std::cout << "Connexion à " << infos.ip << ":" << port << "...\n";
    if (connect(sock, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0) {
        std::cerr << "Erreur : Connexion au serveur échouée.\n";
        close(sock);
        return 1;
    }

    // 5. Commande du protocole (pseudo choisi sur l'écran de connexion)
    std::string cmd_connect = "CONNECT " + infos.pseudo + "\n";
    send(sock, cmd_connect.c_str(), cmd_connect.size(), 0);

    // 6. Interface du chat
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
        // Le dernier message reste toujours visible (défilement auto)
        if (!msg_elements.empty()) {
            msg_elements.back() = msg_elements.back() | focus;
        }

        std::string titre = " TAP Client TUI - " + infos.pseudo + " @ " +
                            infos.ip + ":" + std::to_string(port) + " ";

        return vbox({
            window(text(titre), vbox(msg_elements) | yframe) | flex,
            hbox({text(" > "), champ->Render()}),
        });
    });

    // Touche Entrée : envoi du message (ou sortie si "QUIT")
    renderer |= CatchEvent([&](Event event) {
        if (event != Event::Return) return false;

        if (saisie == "QUIT") {
            screen.Exit();
            return true;
        }
        if (!saisie.empty()) {
            ajouterMessage("Moi: " + saisie);
            // Le '\n' est obligatoire pour respecter le format du protocole
            std::string ligne = saisie + "\n";
            send(sock, ligne.c_str(), ligne.size(), 0);
            saisie.clear();
        }
        return true;
    });

    // 7. Thread réseau en arrière-plan, puis boucle de l'interface
    std::thread threadReception(ecouterServeur, sock, std::ref(screen));
    screen.Loop(renderer);

    // 8. Nettoyage : shutdown() débloque le recv() du thread (close() seul ne suffit pas)
    interface_active = false;
    shutdown(sock, SHUT_RDWR);
    threadReception.join();
    close(sock);

    return 0;
}
