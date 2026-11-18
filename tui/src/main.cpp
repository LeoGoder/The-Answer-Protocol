// Application entry point and lifecycle orchestration

#include <csignal>
#include <cstring>
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

    // Register termination handlers for terminal close or external signals
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = NetworkClient::handleSignalShutdown;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGHUP, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGQUIT, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);

    ConnectionConfig config;
    std::string error_message;

    // Main session loop: re-opens login view if server connection drops
    while (true) {
        NetworkClient client;

        // Display login modal and establish connection
        if (!showLoginView(client, config, error_message)) {
            std::cout << "Connection canceled.\n";
            return 0;
        }
        error_message.clear();

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
