// Application entry point and lifecycle orchestration

#include <csignal>
#include <iostream>
#include <string>

#include "../includes/app_state.hpp"
#include "../includes/game_view.hpp"
#include "../includes/login_view.hpp"
#include "../includes/models.hpp"
#include "../includes/network_client.hpp"

/**
 * @brief Application entry point orchestrating top-level session lifecycle.
 *
 * Sets up OS signal handlers (SIGPIPE ignore), manages the connection retry loop
 * across socket drops, launches the login modal, and initiates the fullscreen
 * interactive game view loop upon successful server handshake.
 *
 * @return 0 upon clean exit.
 */
int main() {
    // Ignore SIGPIPE to prevent crash on broken socket write
    std::signal(SIGPIPE, SIG_IGN);

    ConnectionConfig config;
    std::string error_message;

    // Main session loop: re-opens login view if server connection drops
    while (true) {
        NetworkClient client;

        // Login retry loop: prompt until connected or canceled
        while (true) {
            // Display login modal
            if (!showLoginView(config, error_message)) {
                std::cout << "Connection canceled.\n";
                return 0;
            }

            // Connect to remote server
            if (client.connectToServer(config, error_message)) {
                error_message.clear();
                break;
            }
        }

        // Initialize fresh game state and launch interactive game interface
        AppState state;
        bool connection_lost = runGameView(client, state, config);

        if (connection_lost) {
            error_message = "Connection lost";
        } else {
            // Exit requested by user
            break;
        }
    }

    return 0;
}
