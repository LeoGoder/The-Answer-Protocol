/**
 * @file game_view.hpp
 * @brief Interactive game screen interface and main UI layout declarations.
 */

#pragma once

#include "../includes/app_state.hpp"
#include "../includes/models.hpp"
#include "../includes/network_client.hpp"

/**
 * @brief Runs the primary interactive game loop using FTXUI.
 *
 * Sets up the 3-column split terminal UI:
 * - Left column: Inventory items and active quest objectives.
 * - Center column: Scrollable terminal message log and command input line.
 * - Right column: TrueColor Unicode room map and multi-channel chat view.
 * Handles keyboard input, history cycling (Up/Down arrows), client-side command
 * shortcuts (QUIT, CLEAR), and starts the background TCP listener thread.
 *
 * @param client Reference to the active NetworkClient socket manager.
 * @param state Shared application state storing game records and terminal messages.
 * @param config Current connection parameters (server IP, port, player pseudo).
 * @return True if the game loop exited unexpectedly due to lost connection, false if user quit.
 */
bool runGameView(NetworkClient& client,
                 AppState& state,
                 const ConnectionConfig& config);
