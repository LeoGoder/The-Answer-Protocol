/**
 * @file login_view.hpp
 * @brief Connection configuration and login modal view declarations.
 */

#pragma once

#include <string>

#include "../includes/models.hpp"

/**
 * @brief Displays the initial interactive login screen to gather connection parameters.
 *
 * Prompts user for Server IP, Port, and Pseudo. Validates that the fields are non-empty
 * and attempts server connection.
 *
 * @param config Output connection configuration populated upon user confirmation.
 * @param initial_error Optional error banner to display if previous connection failed.
 * @return True if user submitted credentials, false if user canceled/quit.
 */
bool showLoginView(ConnectionConfig& config,
                   const std::string& initial_error = "");
