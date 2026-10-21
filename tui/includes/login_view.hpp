#pragma once

#include <string>

#include "../includes/models.hpp"

bool showLoginView(ConnectionConfig& config,
                   const std::string& initial_error = "");

inline bool ecranConnexion(ConnectionConfig& config,
                           const std::string& initial_error = "") {
    return showLoginView(config, initial_error);
}
