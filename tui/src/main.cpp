#include <csignal>
#include <iostream>
#include <string>

#include "../includes/app_state.hpp"
#include "../includes/game_view.hpp"
#include "../includes/login_view.hpp"
#include "../includes/models.hpp"
#include "../includes/network_client.hpp"

int main() {
    std::signal(SIGPIPE, SIG_IGN);

    ConnectionConfig config;
    std::string error_message;

    while (true) {
        NetworkClient client;

        while (true) {
            if (!showLoginView(config, error_message)) {
                std::cout << "Connexion annulée.\n";
                return 0;
            }

            if (client.connectToServer(config, error_message)) {
                error_message.clear();
                break;
            }
        }

        AppState state;
        bool connection_lost = runGameView(client, state, config);

        if (connection_lost) {
            error_message = "Connection lost";
        } else {
            break;
        }
    }

    return 0;
}
