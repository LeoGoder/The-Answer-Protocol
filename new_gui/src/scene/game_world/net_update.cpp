#include "game_world.hpp"
#include "server.hpp"
#include <cstdio>
#include <unistd.h>

void GameWorld::server_alive(std::vector<std::string> msg_split) {
    if (this->alive_timer > this->alive_check_timer) {
        if (send_message(command.ALIVE, "", this->fd) < 0) {
            printf("Error while send_message");
        }
        this->alive_timer = 0.0;
    }
    if (!msg_split.empty()) {
        if (msg_split[0].rfind("OK") == 0 && msg_split[1].rfind("Alive") == 0) {
            this->alive_last_time = 0.0;
        }
        this->alive_last_time += this->game_state->dt;
        if (this->alive_last_time > ALIVE_MAX_TIME){
            this->intern_scene = connection_error;
            close(this->fd);
            this->fd = -1;
        }
    }
}
