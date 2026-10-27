#include "game_world.hpp"
#include "map_parse.hpp"
#include "helper.hpp"
#include "player.hpp"
#include "raylib.h"
#include "server.hpp"
#include "parsing_command_response.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>

GameWorld::GameWorld(GameState *game_state){
    this->font = LoadTexture(GetAssetPath("test_font.png").c_str());
    this->player_sprite = LoadTexture(GetAssetPath("player.png").c_str());
    this->ground = LoadTexture(GetAssetPath("Map.png").c_str());
    this->fox = LoadTexture(GetAssetPath("Fox_Idle_with_shadow.png").c_str());
    this->hare = LoadTexture(GetAssetPath("Hare_Idle_with_shadow.png").c_str());
    this->sheep = LoadTexture(GetAssetPath("Sheep_animation_with_shadow.png").c_str());
    this->calf = LoadTexture(GetAssetPath("Calf_animation_with_shadow.png").c_str());
    this->trees = LoadTexture(GetAssetPath("trees.png").c_str());
    this->rock_flowers = LoadTexture(GetAssetPath("rock_and_flowers.png").c_str());
    this->exterior = LoadTexture(GetAssetPath("exterior.png").c_str());
    this->game_state = game_state;
    this->intern_scene = connection;
    memset(&this->server, 0, sizeof(this->server));
    this->server.sin_family = AF_INET;
    this->server.sin_port = htons(SERVER_PORT);
    this->server.sin_addr.s_addr = inet_addr(SERVER_IP);
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    this->player_name = this->game_state->player_name;
    this->alive_check_timer = 15;
    this->alive_timer = 0.0;
    this->alive_nb_fail = 0;
    this->player = Player(&this->font, "", &this->player_sprite);
    this->player.cam.offset = {float(SCREEN_WIDTH) / 2, float(SCREEN_HEIGHT) / 2};
    this->player.cam.rotation = 0.0;
    this->player.cam.target = this->player.player_pos;
    this->player.cam.zoom = 1.0;
    this->map = hub;
    parse_map(this->map_data);
}

GameWorld::~GameWorld() {
    UnloadTexture(this->font);
    UnloadTexture(this->player_sprite);
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
    if (this->intern_scene == in_game) {
        this->player.input_player(this->game_state->dt, this->map, this->map_data);
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
    std::string buffer;
    if (this->fd > 0)
        buffer = receiv_msg(this->fd);
    if (this->fd == 0) {
        close(fd);
        this->fd = -1;
        this->intern_scene = connection_error;
    }
    if (buffer != "") {
        kill_return_line(buffer);
        msg_copy = buffer;
        msg_split = split(buffer, ' ', 1);
    }
    switch (this->intern_scene) {
        case connection:
            break;
        case connection_loading:
            if (!msg_split.empty()) {
                if (is_connection_approved(msg_split) == true) {
                    this->intern_scene = in_game;
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
        case in_game:
            if (!msg_split.empty()) {
                if (msg_copy.rfind(R"(OK {"room")") == 0) {
                    parse_look_response(msg_split, this->room, &this->font, &this->player_sprite);
                }
                if (msg_copy.rfind(R"(OK room=)") == 0) {
                    parse_move_response(msg_split, this->room, this->command, this->fd);
                }
                if (msg_copy.rfind(R"(OK )" + this->event.ROOM_ENTER) == 0) {
                    std::string new_player = split(msg_split[1], ' ')[3];
                    GameWorld::room_presence_enter(new_player);
                if (msg_copy.rfind(R"(OK )" + this->event.ROOM_LEAVE) == 0) {
                    std::string del_player = split(msg_split[1], ' ')[3];
                    GameWorld::room_presence_leave(del_player);
                }
            }
            std::string p_pos = "";
            p_pos += std::to_string(int(this->player.player_pos.x)) + " " + std::to_string(int(this->player.player_pos.y)) + '\n';
            if (int(this->alive_timer) % 24 == 0)
                send_message(this->command.POSITION, p_pos, this->fd);
            break;
            }
    }

    GameWorld::server_alive(msg_split);
    this->alive_timer += this->game_state->dt;
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
        case in_game:
            float player_center = (float(this->player_sprite.width) / 2);

            this->player.cam.target.x = this->player.player_pos.x + player_center;
            this->player.cam.target.y = this->player.player_pos.y;
            BeginMode2D(this->player.cam);
                draw_game();
            EndMode2D();
            draw_hud();
            break;
    }
    GameWorld::net_update();
}
