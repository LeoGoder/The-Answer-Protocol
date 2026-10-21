#include "game_world.hpp"
#include "helper.hpp"
#include "raylib.h"
#include "server.hpp"
#include "parsing_command_response.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>

GameWorld::GameWorld(GameState *game_state) {
    this->font = LoadTexture(GetAssetPath("test_font.png").c_str());
    this->game_state = game_state;
    this->intern_scene = connection;

    memset(&this->server, 0, sizeof(this->server));
    this->server.sin_family = AF_INET;
    this->server.sin_port = htons(SERVER_PORT);
    this->server.sin_addr.s_addr = inet_addr(SERVER_IP);
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    this->player_name = this->game_state->player_name;
}

GameWorld::~GameWorld() {
    UnloadTexture(this->font);
    if (this->fd >= 0) {
        close(this->fd);
        this->fd = -1;
    }
}

void GameWorld::update() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        game_state->set_init_scene(true);
        game_state->set_actual_scene(splash_screen);
    }
}

static bool is_connection_approved(std::vector<std::string> msg) {
    std::vector<std::string> args_split = split(msg[1], ' ');

    for (long unsigned int i = 0; i < args_split.size(); i++) {
        if (args_split[i] == "OK" && args_split[i + 1] == "connected") {
            return true;
        }
    }
    return false;
}

void GameWorld::net_update() {
    std::vector<std::string> msg_split;
    std::string msg_copy;
    std::string buffer = receiv_msg(this->fd);
    if (buffer != "") {
        kill_return_line(buffer);
        msg_copy = buffer;
        msg_split = split(buffer, ' ', 1);
        // std::cout << msg_split[0];
    }
    switch (this->intern_scene) {
        case connection:
            break;
        case connection_loading:
            if (!msg_split.empty()) {
                if (is_connection_approved(msg_split) == true) {
                    this->intern_scene = island_1;
                    int res = send_message(this->command.LOOK, "", this->fd);

                    if (res < 0) {
                        // TODO: change to notify system for ux enhanced
                        std::cout << "Error send\n";
                    }
                }
                else {
                    this->intern_scene = connection_error;
                    this->error_log = msg_copy;
                }
            }
            break;
        case connection_error:
            break;
        case island_1:
            if (!msg_split.empty()) {
                if (msg_split[0] == "OK" && msg_split[1].front() == '{') {
                    parse_look_response(msg_split, this->room);
                }
            }
            break;
    }
}

void GameWorld::draw() {
    switch (this->intern_scene) {
        case connection: {
            if (this->fd < 0) {
                this->intern_scene = connection_error;
                break;
            }

            int flags = fcntl(this->fd, F_GETFL, 0);
            fcntl(this->fd, F_SETFL, flags | O_NONBLOCK);
            int cret = connect(this->fd, (struct sockaddr *)&this->server, sizeof(this->server));
            bool connected = false;

            if (cret == 0) {
                connected = true;
            } else if (errno == EINPROGRESS) {
                fd_set fdset;
                struct timeval tv = { .tv_sec = 0, .tv_usec = 100000 };

                FD_ZERO(&fdset);
                FD_SET(this->fd, &fdset);

                if (select(this->fd + 1, NULL, &fdset, NULL, &tv) > 0) {
                    int so_err = 0;
                    socklen_t len = sizeof(so_err);
                    getsockopt(this->fd, SOL_SOCKET, SO_ERROR, &so_err, &len);
                    if (so_err == 0) {
                        connected = true;
                    }
                }
            }
            if (connected) {
                int res = send_message(this->command.CONNECT, this->player_name, this->fd);
                if (res > 0) {
                    this->intern_scene = connection_loading;
                }
            }
            else {
                close(this->fd);
                this->fd = -1;
                this->intern_scene = connection_error;
            }
            break;
        }
        case connection_loading:
            draw_loading_screen();
            break;
        case connection_error:
            draw_connection_error();
            break;
        case island_1:
            draw_island_1();
            break;
    }
    GameWorld::net_update();
}
