/**
 * @file login_view.hpp
 * @brief Connection configuration and login modal view declarations.
 */

#pragma once

#include <string>

#include "../includes/models.hpp"
#include "../includes/network_client.hpp"

/**
 * @brief Displays the initial interactive login screen to gather connection parameters.
 *
 * Prompts user for Server IP, Port, and Pseudo. Performs in-modal connection
 * with live status and timeout detection.
 *
 * @param client Active network client instance used to establish connection.
 * @param config Output connection configuration populated upon user confirmation.
 * @param initial_error Optional error banner to display if previous connection failed.
 * @return True if connected and authenticated, false if user canceled/quit.
 */
bool showLoginView(NetworkClient& client,
                   ConnectionConfig& config,
                   const std::string& initial_error = "");
