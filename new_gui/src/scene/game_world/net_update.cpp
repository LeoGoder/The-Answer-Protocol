#include "game_world.hpp"
#include "server.hpp"
#include <cstdio>

void GameWorld::server_alive() {
    if (send_message(command.ALIVE, "", this->fd) < 0) {
        printf("Error while send_message");
    }
}
