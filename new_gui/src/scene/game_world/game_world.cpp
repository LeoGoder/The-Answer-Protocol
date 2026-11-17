#include "inc/game_world.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "inc/helper.hpp"
#include "inc/map_parse.hpp"
#include "inc/parsing_command_response.hpp"
#include "inc/player.hpp"
#include "inc/raylib.h"
#include "inc/server.hpp"

GameWorld::GameWorld(GameState *game_state) {
    this->font = LoadTexture(GetAssetPath("test_font.png").c_str());
    SetTextureFilter(this->font, TEXTURE_FILTER_POINT);
    this->player_sprite = LoadTexture(GetAssetPath("player.png").c_str());
    this->ground = LoadTexture(GetAssetPath("Map.png").c_str());
    this->fox = LoadTexture(GetAssetPath("Fox_Idle_with_shadow.png").c_str());
    this->hare = LoadTexture(GetAssetPath("Hare_Idle_with_shadow.png").c_str());
    this->sheep =
        LoadTexture(GetAssetPath("Sheep_animation_with_shadow.png").c_str());
    this->calf =
        LoadTexture(GetAssetPath("Calf_animation_with_shadow.png").c_str());
    this->trees = LoadTexture(GetAssetPath("trees.png").c_str());
    this->rock_flowers =
        LoadTexture(GetAssetPath("rock_and_flowers.png").c_str());
    this->exterior = LoadTexture(GetAssetPath("exterior.png").c_str());
    this->game_state = game_state;
    this->intern_scene = connection;
    memset(&this->server, 0, sizeof(this->server));
    this->server.sin_family = AF_INET;
    this->server.sin_port = htons(SERVER_PORT);
    this->server.sin_addr.s_addr = inet_addr(SERVER_IP);
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    this->player_name = this->game_state->player_name;
    this->alive_timer = 0.0;
    this->alive_last_time = 0.0;
    this->timer = 0.0;
    this->server_players = 0;
    this->hp = 0;
    this->max_hp = 0;
    this->wait_quests = true;
    this->log_quests = false;
    this->log_scroll = 0;
    this->panel_tab = PANEL_LOG;
    this->group_unread = false;
    for (int i = 0; i < 3; i++)
        this->chat_unread[i] = false;
    this->inviting = false;
    this->wait_talk = false;
    this->player = Player(&this->font, "", &this->player_sprite);
    this->player.cam.offset = {float(SCREEN_WIDTH) / 2,
                               float(SCREEN_HEIGHT) / 2};
    this->player.cam.rotation = 0.0;
    this->player.cam.target = this->player.player_pos;
    this->player.cam.zoom = 1.0;
    this->map = hub;
    parse_map(this->map_data);
}

GameWorld::~GameWorld() {
    UnloadTexture(this->font);
    UnloadTexture(this->player_sprite);
    for (std::map<std::string, Texture2D>::iterator it =
             this->object_textures.begin();
         it != this->object_textures.end(); it++)
        UnloadTexture(it->second);
    if (this->fd >= 0) {
        close(this->fd);
        this->fd = -1;
    }
}

void GameWorld::update() {
    if (IsKeyPressed(KEY_ESCAPE) && !this->player.show_chat) {
        game_state->set_init_scene(true);
        game_state->set_actual_scene(splash_screen);
    }
    if (this->intern_scene == in_game) {
        this->player.input_player(this->game_state->dt, this->map,
                                  this->map_data, this->fd);
        // show_chat: the player is typing in the CHAT tab (ESC cancels, Enter
        // sends).
        if (this->player.show_chat) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                this->player.show_chat = false;
            } else if (this->player.chat.chat_input()) {
                GameWorld::send_chat();
                this->player.show_chat = false;
            }
        } else if (IsKeyPressed(KEY_SLASH)) {
            GameWorld::open_chat();
        }
        if (!this->player.show_chat) {
            bool left = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

            if (left && !this->hovered_item.empty())
                send_message(this->command.TAKE, this->hovered_item, this->fd);
            if (left && !this->hovered_npc.empty()) {
                this->dialog_npc = this->hovered_npc;
                this->dialog_text.clear();
                this->wait_talk = true;
                send_message(this->command.TALK, this->hovered_npc, this->fd);
            }
            if (left && !this->hovered_slot.empty() &&
                std::find(this->usable.begin(), this->usable.end(),
                          this->hovered_slot) != this->usable.end())
                send_message(this->command.USE, this->hovered_slot, this->fd);
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) &&
                !this->hovered_slot.empty())
                send_message(this->command.DROP, this->hovered_slot, this->fd);
            if (left && !this->hovered_exit.empty()) {
                send_message("MOVE ", this->hovered_exit, this->fd);
                this->player.player_pos = PLAYER_DEFAULT_POS;
            }
        }
    }
}

void GameWorld::net_update() {
    std::vector<std::string> msg_split;
    std::string msg_copy;
    std::string buffer;
    std::string look;

    if (this->fd > 0) {
        // Only complete lines are read: the end of a long line can come in the
        // next recv.
        this->pending += receiv_msg(this->fd);
        size_t end = this->pending.rfind('\n');

        if (end != std::string::npos) {
            buffer = this->pending.substr(0, end + 1);
            this->pending.erase(0, end + 1);
        }
        // The last LOOK line of the buffer: the "OK" replies to POSITION often
        // come before it.
        size_t look_pos = ("\n" + buffer).rfind("\n" R"(OK {"room")");

        if (look_pos != std::string::npos)
            look =
                buffer.substr(look_pos, buffer.find('\n', look_pos) - look_pos);
    }
    if (this->fd == 0) {
        close(fd);
        this->fd = -1;
        this->intern_scene = connection_error;
    }
    if (buffer != "") {
        if (this->wait_talk)
            GameWorld::read_talk_reply(buffer);
        GameWorld::read_chat(buffer);
        kill_return_line(buffer);
        msg_copy = buffer;
        msg_split = split(buffer, ' ', 1);
    }
    switch (this->intern_scene) {
        case connection:
            break;
        case connection_loading:
            if (!msg_split.empty()) {
                // "OK hello" can arrive alone before "OK connected": wait until
                // one of them or an ERR.
                if (msg_copy.find("OK connected") != std::string::npos) {
                    this->intern_scene = in_game;
                    int res = send_message(this->command.LOOK, "", this->fd);

                    if (res < 0) {
                        // TODO: change to notify system for ux enhanced
                        std::cout << "Error send\n";
                    }
                    send_message(this->command.WHO, "", this->fd);
                    send_message(this->command.INVENTORY, "", this->fd);
                    send_message(this->command.WORLD, "", this->fd);
                    send_message(this->command.STATUS, "", this->fd);
                    send_message(this->command.QUESTS, "", this->fd);
                } else if (msg_copy.find("ERR") != std::string::npos) {
                    this->intern_scene = connection_error;
                    this->error_log = msg_copy;
                }
            }
            break;
        case connection_error:
            break;
        case in_game:
            if (!msg_split.empty()) {
                if (!look.empty()) {
                    std::vector<std::string> look_split = split(look, ' ', 1);

                    parse_look_response(look_split, this->room, &this->font,
                                        &this->player_sprite, this->map);
                    GameWorld::add_log(look);
                }
                size_t move = msg_copy.find("OK room=");
                if (move != std::string::npos) {
                    parse_move_response(this->player_name, this->room,
                                        this->command, this->fd);
                }
                size_t enter_pos = msg_copy.find("EVT ROOM PRESENCE ENTER ");
                if (enter_pos != std::string::npos) {
                    std::string rest = msg_copy.substr(enter_pos);
                    std::vector<std::string> parts = split(rest, ' ');

                    if (parts.size() >= 5)
                        GameWorld::room_presence_enter(parts[4]);
                }
                size_t leave_pos = msg_copy.find("EVT ROOM PRESENCE LEAVE ");
                if (leave_pos != std::string::npos) {
                    std::string rest = msg_copy.substr(leave_pos);
                    std::vector<std::string> parts = split(rest, ' ');

                    if (parts.size() >= 5)
                        GameWorld::room_presence_leave(parts[4]);
                }
                if (msg_copy.find("EVT ROOM POSITION {") != std::string::npos) {
                    update_player_pos(this->room.player, msg_copy);
                }
                GameWorld::update_server_players(msg_copy);
                GameWorld::update_hp(msg_copy);
                // QUESTS replies "OK [{...}]", INVENTORY "OK ["..."]": only the
                // empty "OK []" needs wait_quests.
                size_t quests_pos = msg_copy.find("OK [{");
                size_t inventory_pos = msg_copy.find("OK [\"");
                size_t empty_pos = msg_copy.find("OK []");
                if (this->wait_quests && quests_pos == std::string::npos)
                    quests_pos = empty_pos;
                else
                    inventory_pos = std::min(inventory_pos, empty_pos);
                if (quests_pos != std::string::npos) {
                    std::string reply = msg_copy.substr(quests_pos);

                    this->quests = parse_quests(reply);
                    if (this->log_quests)
                        GameWorld::add_log(reply);
                    this->wait_quests = false;
                    this->log_quests = false;
                }
                if (inventory_pos != std::string::npos) {
                    std::string reply = msg_copy.substr(inventory_pos);

                    this->player.set_inventory(parse_id_list(reply, "OK ["));
                }
                if (msg_copy.find(R"(OK {"hostile")") != std::string::npos) {
                    this->hostile = parse_id_list(msg_copy, R"("hostile":[)");
                    this->quest_givers = parse_id_list(msg_copy, R"("quest_givers":[)");
                    this->usable = parse_id_list(msg_copy, R"("usable":[)");
                }
                size_t quest_pos = msg_copy.find(R"(OK {"quest_id")");
                if (quest_pos != std::string::npos)
                    GameWorld::add_log(msg_copy.substr(quest_pos));
                // A completed quest raises HP and takes the delivered item,
                // without telling it.
                if (quest_pos != std::string::npos &&
                    msg_copy.find(R"("status":"completed")", quest_pos) !=
                        std::string::npos) {
                    send_message(this->command.STATUS, "", this->fd);
                    send_message(this->command.INVENTORY, "", this->fd);
                }
                // A defeated or respawned enemy changes the NPCs of the room.
                if (msg_copy.find(R"("status":"victory")") !=
                        std::string::npos ||
                    msg_copy.find("EVT ROOM NPC SPAWN ") != std::string::npos) {
                    send_message(this->command.LOOK, "", this->fd);
                    send_message(this->command.INVENTORY, "", this->fd);
                }
                if (msg_copy.find(R"("status":"defeated")") != std::string::npos) {
                    send_message(this->command.LOOK, "", this->fd);
                    this->player.player_pos = PLAYER_DEFAULT_POS;
                }
                size_t attack_pos = msg_copy.find(R"(OK {"attacker")");
                if (attack_pos != std::string::npos)
                    GameWorld::add_log(msg_copy.substr(attack_pos));
                size_t status_pos = msg_copy.find(R"(OK {"hp")");
                if (status_pos != std::string::npos)
                    GameWorld::add_log(msg_copy.substr(status_pos));
                size_t who_pos = msg_copy.find("OK players=");
                if (who_pos != std::string::npos)
                    GameWorld::add_log(msg_copy.substr(who_pos));
                size_t err_pos = msg_copy.find("ERR ");
                if (err_pos != std::string::npos)
                    GameWorld::add_log(msg_copy.substr(err_pos));
                GameWorld::update_items(msg_copy);
                GameWorld::update_group(msg_copy);
                GameWorld::refresh_quests(msg_copy);
            }
            this->timer += this->game_state->dt;
            if (this->timer >= REQUEST_FREQUENCE) {
                this->timer = 0.0;
                std::string p_pos = "";

                p_pos += std::to_string(int(this->player.player_pos.x)) + " " +
                         std::to_string(int(this->player.player_pos.y)) + '\n';
                send_message(this->command.POSITION, p_pos, this->fd);
            }
            break;
            }

    GameWorld::server_alive(msg_copy);
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
            int cret = connect(this->fd, (struct sockaddr *)&this->server,
                               sizeof(this->server));
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
                int res = send_message(this->command.CONNECT, this->player_name,
                                       this->fd);
                if (res > 0) {
                    this->intern_scene = connection_loading;
                }
            } else {
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
            // float player_center = (float(this->player_sprite.width) / 2);

            this->player.cam.target.x =
                this->player.player_pos.x + SPRITE_SIZE.x;
            this->player.cam.target.y = this->player.player_pos.y;
            BeginMode2D(this->player.cam);
                draw_game();
            EndMode2D();
            draw_hud();
            break;
    }
    GameWorld::net_update();
}
