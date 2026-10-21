#pragma once

#include "../includes/app_state.hpp"
#include "../includes/models.hpp"
#include "../includes/network_client.hpp"

void runGameView(NetworkClient& client,
                 AppState& state,
                 const ConnectionConfig& config);
