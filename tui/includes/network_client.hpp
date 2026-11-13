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
     * @brief Background loop continuously receiving TCP packets and framing newline-terminated lines.
     *
     * Frames stream into lines, delegates each line to processIncomingLine(),
     * and signals FTXUI screen redraws.
     *
     * @param screen FTXUI screen reference to receive refresh events.
     * @param state Central application state.
     */
    void listenerLoop(ftxui::ScreenInteractive& screen, AppState& state);

    // Dispatches a single framed protocol line across specialized handlers
    void processIncomingLine(const std::string& line, AppState& state,
                             ftxui::ScreenInteractive& screen);

    // Updates UI room description and resolves map asset
    void updateRoomState(const RoomInfo& room, AppState& state);

    // Handles server responses to CMD_MOVE (parsing room JSON or triggering LOOK fallback)
    bool handleMoveResponse(const std::string& line, AppState& state);

    // Handles server responses to TAKE, DROP, and INVENTORY queries
    bool handleInventoryResponse(const std::string& line, int cmd,
                                 AppState& state);

    // Handles item consumption responses and triggers automatic inventory update
    bool handleUseResponse(const std::string& line, AppState& state);

    // Handles attack replies, formats combat results, and triggers LOOK on victory
    bool handleAttackResponse(const std::string& line, AppState& state);

    /**
     * @brief Deserializes and formats structured JSON responses for LOOK, QUEST, and QUESTS.
     *
     * Also provides fallback flattening for arbitrary generic JSON responses.
     */
    bool handleJsonQueryResponse(const std::string& line, int cmd,
                                 AppState& state);

    // Handles bare OK / ERR acknowledgments for optimistic in-flight chat messages
    bool handleChatAck(const std::string& line, int cmd, AppState& state);

    // Parses asynchronous server broadcast events (combat notices and multi-channel chat)
    bool handleAsyncEvent(const std::string& line, AppState& state);

    int socket_fd_;
    std::mutex send_mutex_;
    std::unique_ptr<std::thread> listener_thread_;
    std::atomic<bool> connection_lost_{false};
    std::atomic<bool> user_quit_{false};
};
