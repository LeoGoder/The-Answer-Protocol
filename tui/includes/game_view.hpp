#pragma once

#include "../includes/app_state.hpp"
#include "../includes/models.hpp"
#include "../includes/network_client.hpp"

bool runGameView(NetworkClient& client,
                 AppState& state,
                 const ConnectionConfig& config);
