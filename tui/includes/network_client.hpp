/**
 * @file network_client.hpp
 * @brief TCP networking client and asynchronous protocol listener declarations.
 */

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <ftxui/component/screen_interactive.hpp>

#include "../includes/app_state.hpp"
#include "../includes/models.hpp"

/**
 * @class NetworkClient
 * @brief Manages TCP socket communication and asynchronous message reception.
 *
 * Provides thread-safe methods to connect, authenticate with the TAP server,
 * transmit commands, and run a dedicated background receiver thread that parses
 * line-delimited responses and signals the FTXUI event loop.
 */
class NetworkClient {
 public:
    // Default constructor initializing socket file descriptor and atomic flags
    NetworkClient();

    // Destructor ensuring socket and listener thread are cleanly closed
    ~NetworkClient();

    /**
     * @brief Connects to the remote TAP server and performs the initial authentication handshake.
     *
     * Resolves host address, opens TCP socket, and transmits "CONNECT <pseudo>\n".
     * Reads initial greeting banner and server OK confirmation.
     *
     * @param config Target connection parameters (host IP, port, pseudo).
     * @param error_message Output string populated if connection or authentication fails.
     * @return True if connected and authenticated successfully, false otherwise.
     */
    bool connectToServer(const ConnectionConfig& config,
                         std::string& error_message);

    // Closes the TCP socket connection and terminates background listener
    void disconnect();

    // Checks whether an active socket connection is currently open
    bool isConnected() const;

    // Checks whether the connection was severed unexpectedly by the remote host
    bool isConnectionLost() const;

    // Sets flag indicating whether disconnection was initiated voluntarily by the user
    void setUserQuit(bool val);

    // Sends a line of text terminated by a newline character across the socket
    bool sendLine(const std::string& line);

    /**
     * @brief Spawns the background listener thread.
     *
     * @param screen FTXUI interactive screen to post Custom redraw events.
     * @param state Shared application state updated with received data.
     */
    void startListener(ftxui::ScreenInteractive& screen, AppState& state);

    // Signals and joins the background listener thread if active
    void stopListener();

 private:
    /**
     * @brief Background loop continuously receiving TCP packets and dispatching protocol lines.
     *
     * Frames stream into lines, routes responses to matching pending commands,
     * updates AppState, and signals screen redraw.
     *
     * @param screen FTXUI screen reference to receive refresh events.
     * @param state Central application state.
     */
    void listenerLoop(ftxui::ScreenInteractive& screen, AppState& state);

    int socket_fd_;
    std::mutex send_mutex_;
    std::unique_ptr<std::thread> listener_thread_;
    std::atomic<bool> connection_lost_{false};
    std::atomic<bool> user_quit_{false};
};
