#include "game_world.hpp"
#include "helper.hpp"
#include "raylib.h"
#include <cstddef>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

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
                ssize_t res = send(this->fd, msg.data(), msg.size(), 0);
                
                if (res >= 0) {
                    this->intern_scene = island_1;
                    break;
                }
            }
            else {
                close(this->fd);
                this->fd = -1;
                this->intern_scene = connection_error;
            }
            break;
        }
        case connection_error:
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(RED, 0.30));
            DrawFontText(this->font, "Connection issue !", 20, 20, WHITE, 1.0f);
            DrawFontText(this->font, "Press ESC to return", 20, 50, Color{180, 180, 180, 255}, 1.0f);
            break;
        case island_1:
            DrawFontText(this->font, "Connection working !", 20, 20, WHITE, 1.0f);
            DrawFontText(this->font, "Press ESC to return", 20, 50, Color{180, 180, 180, 255}, 1.0f);
            break;
    }
}
