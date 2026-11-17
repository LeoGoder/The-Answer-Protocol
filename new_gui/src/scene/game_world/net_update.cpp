#include "game_world.hpp"
#include "server.hpp"
#include <cstdio>
#include <unistd.h>

// "OK Alive" is searched in the whole buffer: the "OK" replies to POSITION
// often arrive in the same recv.
void GameWorld::server_alive(std::string msg) {
    if (this->alive_timer > ALIVE_CHECK_TIMER) {
        if (send_message(command.ALIVE, "", this->fd) < 0) {
            printf("Error while send_message");
        }
        this->alive_timer = 0.0;
    }
    if (msg.find("OK Alive") != std::string::npos)
        this->alive_last_time = 0.0;
    this->alive_last_time += this->game_state->dt;
    if (this->alive_last_time > ALIVE_MAX_TIME){
        this->intern_scene = connection_error;
        close(this->fd);
        this->fd = -1;
    }
}
