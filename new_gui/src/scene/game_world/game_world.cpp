#include "game_world.hpp"
#include "helper.hpp"
#include <cstring>
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

            if (cret == 0) {
                this->intern_scene = island_1;
            } else if (errno == EINPROGRESS) {
                fd_set fdset;
                struct timeval tv;
                FD_ZERO(&fdset);
                FD_SET(this->fd, &fdset);
                tv.tv_sec = 0;
                tv.tv_usec = 100000;

                int sel = select(this->fd + 1, NULL, &fdset, NULL, &tv);
                if (sel > 0) {
                    int so_err = 0;
                    socklen_t len = sizeof(so_err);
                    getsockopt(this->fd, SOL_SOCKET, SO_ERROR, &so_err, &len);
                    if (so_err == 0) {
                        this->intern_scene = island_1;
                    } else {
                        close(this->fd);
                        this->fd = -1;
                        this->intern_scene = connection_error;
                    }
                } else {
                    close(this->fd);
                    this->fd = -1;
                    this->intern_scene = connection_error;
                }
            } else {
                close(this->fd);
                this->fd = -1;
                this->intern_scene = connection_error;
            }
            break;
        }
        case connection_error:
            DrawFontText(this->font, "Connection issue !", 20, 20, WHITE, 1.0f);
            DrawFontText(this->font, "Press ESC to return", 20, 50, Color{180, 180, 180, 255}, 1.0f);
            break;
        case island_1:
            DrawFontText(this->font, "Connection working !", 20, 20, WHITE, 1.0f);
            DrawFontText(this->font, "Press ESC to return", 20, 50, Color{180, 180, 180, 255}, 1.0f);
            break;
    }
}
