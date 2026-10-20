#include "game_world.hpp"
#include "helper.hpp"
#include "raylib.h"
#include "server.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
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
    for (long unsigned int i = 0; i < msg.size(); i++) {
        if (msg[i] == "OK" && msg[i + 1] == "connected") {
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
        msg_split = split(buffer, ' ');
    }
    switch (this->intern_scene) {
        case connection:
            break;
        case connection_loading:
            if (!msg_split.empty()) {
                if (is_connection_approved(msg_split) == true)
                    this->intern_scene = island_1;
                else {
                    this->intern_scene = connection_error;
                    this->error_log = msg_copy;
                }
            }
            break;
        case connection_error:
            break;
        case island_1:
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
                std::string msg = this->command.CONNECT + this->game_state->player_name + '\n';
                ssize_t res = send(this->fd, msg.data(), strlen(msg.data()), 0);
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
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(GREEN, 0.30));
            DrawFontText(this->font, "Connecting...", 20, 20, WHITE, 1.0f);
            break;
        case connection_error:
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(RED, 0.30));
            DrawFontText(this->font, "Connection issue !", 20, 20, WHITE, 1.0f);
            DrawFontText(this->font, this->error_log, 20, 50, WHITE, 1.0f);
            DrawFontText(this->font, "Press ESC to return", 20, 80, Color{180, 180, 180, 255}, 1.0f);
            break;
        case island_1:
            DrawFontText(this->font, "Connection working !", 20, 20, WHITE, 1.0f);
            DrawFontText(this->font, "Press ESC to return", 20, 50, Color{180, 180, 180, 255}, 1.0f);
            GameWorld::server_alive();
            break;
    }
    GameWorld::net_update();
}
