#include "player.hpp"
#include "chat.hpp"
#include "helper.hpp"
#include "map_parse.hpp"
#include "raylib.h"
#include "server.hpp"
#include <cstddef>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

void update_player_pos(std::vector<std::unique_ptr<Player>> &vect_players, std::string event) {
    for (size_t i = 0; i < vect_players.size(); i++) {
        std::string new_position = extract_position(event, vect_players[i]->name);
        std::vector<std::string> position_split;

        if (new_position == "")
            continue;
        position_split = split(new_position, ',');
        vect_players[i]->player_pos.x = std::stof(position_split[0]);
        vect_players[i]->player_pos.y = std::stof(position_split[1]);
    }
}

Player::Player(Texture2D *font, std::string name, Texture2D *sprite) : chat() {
    this->player_pos = PLAYER_DEFAULT_POS;
    this->last_player_pos = this->player_pos;
    this->font = font;
    this->name = name;
    this->sprite = sprite;
    this->show_chat = false;
    this->time = 0.0f;
    this->idle_animation = 0;
    this->walk_animation = 0;
    this->last_direction = 0;
}

Player::Player() : chat() {
    (void)font;
}

Player::~Player() {

}

void Player::update_animation_timer(float dt) {
    this->time += dt;
    if (this->time > TARGET_SPEED) {
        this->time = 0.0f;

        this->idle_animation += 1;
        if (this->idle_animation >= PLAYER_IDLE_ANIMATION) {
            this->idle_animation = 0;
        }

        this->walk_animation += 1;
        if (this->walk_animation >= PLAYER_WALKING_ANIMATION) {
            this->walk_animation = 0;
        }
    }
}

void Player::draw_other_player(std::string player_name, float dt) {
    float player_center = 32;
    float text_len_full = float(GetFontTextWidth(player_name));
    float text_len = float(GetFontTextWidth(player_name) / 2.0f - (float(CHAR_WIDTH) / 2));

    DrawRectangle(this->player_pos.x - text_len + player_center - (float(CHAR_WIDTH) / 4), this->player_pos.y - 20 - (float(CHAR_WIDTH) / 6), text_len_full, CHAR_HEIGHT, ColorAlpha(BLACK, 0.6));
    DrawFontText(*this->font, player_name, this->player_pos.x - text_len + player_center, this->player_pos.y - 20);

    if ((this->last_player_pos.x > this->player_pos.x) && (this->last_player_pos.y > this->player_pos.y)) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_LEFT_DIAG_UP, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_LEFT_DIAG_UP;
    }
    else if ((this->last_player_pos.x > this->player_pos.x) && (this->last_player_pos.y < this->player_pos.y)) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_LEFT_DIAG_DOWN, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_LEFT_DIAG_DOWN;
    }
    else if ((this->last_player_pos.x < this->player_pos.x) && (this->last_player_pos.y > this->player_pos.y)) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_RIGHT_DIAG_UP, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_RIGHT_DIAG_UP;
    }
    else if ((this->last_player_pos.x < this->player_pos.x) && (this->last_player_pos.y < this->player_pos.y)) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_RIGHT_DIAG_DOWN, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_RIGHT_DIAG_DOWN;
    }

    else if (this->last_player_pos.x > this->player_pos.x) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_LEFT, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_LEFT;
    }
    else if (this->last_player_pos.x < this->player_pos.x) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_RIGHT, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_RIGHT;
    }
    else if (this->last_player_pos.y > this->player_pos.y) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_BACK, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_BACK;
    }
    else if (this->last_player_pos.y < this->player_pos.y) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_FRONT, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_FRONT;
    }
    else {
        int direction;

        if (this->last_direction > 0)
            direction = this->last_direction - (8 * 6);
        else
            direction = 0;
        draw_sprite_sheet(*this->sprite, this->idle_animation + direction, 6, this->player_pos, SPRITE_SIZE, 2.0f);
    }
    this->last_player_pos = this->player_pos;
    update_animation_timer(dt);
}

void Player::draw_player(std::string player_name, float dt) {
    float player_center = 32;
    float text_len_full = float(GetFontTextWidth(player_name));
    float text_len = float(GetFontTextWidth(player_name) / 2.0f - (float(CHAR_WIDTH) / 2));

    DrawRectangle(this->player_pos.x - text_len + player_center - (float(CHAR_WIDTH) / 4), this->player_pos.y - 20 - (float(CHAR_WIDTH) / 6), text_len_full, CHAR_HEIGHT, ColorAlpha(BLACK, 0.6));
    DrawFontText(*this->font, player_name, this->player_pos.x - text_len + player_center, this->player_pos.y - 20);

    if (IsKeyDown(KEY_A) && IsKeyDown(KEY_W)) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_LEFT_DIAG_UP, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_LEFT_DIAG_UP;
    }
    else if (IsKeyDown(KEY_A) && IsKeyDown(KEY_S)) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_LEFT_DIAG_DOWN, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_LEFT_DIAG_DOWN;
    }
    else if (IsKeyDown(KEY_D) && IsKeyDown(KEY_W)) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_RIGHT_DIAG_UP, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_RIGHT_DIAG_UP;
    }
    else if (IsKeyDown(KEY_D) && IsKeyDown(KEY_S)) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_RIGHT_DIAG_DOWN, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_RIGHT_DIAG_DOWN;
    }

    else if (IsKeyDown(KEY_A)) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_LEFT, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_LEFT;
    }
    else if (IsKeyDown(KEY_D)) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_RIGHT, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_RIGHT;
    }
    else if (IsKeyDown(KEY_W)) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_BACK, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_BACK;
    }
    else if (IsKeyDown(KEY_S)) {
        draw_sprite_sheet(*this->sprite, this->walk_animation + WALK_FRONT, 6, this->player_pos, SPRITE_SIZE, 2.0f);
        this->last_direction = WALK_FRONT;
    }
    else {
        int direction;

        if (this->last_direction > 0)
            direction = this->last_direction - (8 * 6);
        else
            direction = 0;
        draw_sprite_sheet(*this->sprite, this->idle_animation + direction, 6, this->player_pos, SPRITE_SIZE, 2.0f);
    }
    update_animation_timer(dt);
}

bool player_overlap(std::vector<std::vector<int>> &data, Vector2 player_pos) {
    if (data.empty()) return false;

    int max_x = TILE_WIDTH * MAP_WIDTH;
    size_t layer = data.size() - 2;
    Vector2 tile_pos = {0.0, 0.0};

    for (size_t y = 0; y < data[layer].size(); y++) {
        if (tile_pos.x >= max_x) {
            tile_pos.x = 0.0;
            tile_pos.y += TILE_HEIGHT;
        }
        if (data[layer][y] != 0) {
            if ((player_pos.x + PLAYER_WIDTH >= tile_pos.x - PLAYER_OFFSET && player_pos.x + PLAYER_OFFSET <= tile_pos.x + TILE_WIDTH) &&
                (player_pos.y + PLAYER_HEIGHT >= tile_pos.y - PLAYER_OFFSET && player_pos.y + PLAYER_OFFSET <= tile_pos.y + TILE_HEIGHT)) {
                return true;
            }
        }
        tile_pos.x += TILE_WIDTH;
    }
    return false;
}

bool can_move(MapData &data, Maps &map, Vector2 player_pos) {
    switch (map) {
        case hub:
            return !player_overlap(data.hub, player_pos);
        case fejka:
            return !player_overlap(data.fejka, player_pos);
            break;
        case jeetomal:
            return !player_overlap(data.jeetomal, player_pos);
            break;
        case komolexos:
            return !player_overlap(data.komolexos, player_pos);
            break;
        case kuggis:
            return !player_overlap(data.kuggis, player_pos);
            break;
        case smycka:
            return !player_overlap(data.smycka, player_pos);
            break;
        case vinterfint:
            return !player_overlap(data.vinterfint, player_pos);
            break;
        case fugoumitak:
            return !player_overlap(data.fugoumitak, player_pos);
            break;
    }

    return false;
}

std::string player_change_room(std::vector<std::vector<int>> &data, Vector2 player_pos) {
    if (data.empty()) return "";

    int max_x = TILE_WIDTH * MAP_WIDTH;
    size_t layer = data.size() - 1;
    Vector2 tile_pos = {0.0, 0.0};

    for (size_t y = 0; y < data[layer].size(); y++) {
        int tile = data[layer][y];

        if (tile_pos.x >= max_x) {
            tile_pos.x = 0.0;
            tile_pos.y += TILE_HEIGHT;
        }
        if (tile != 0) {
            if ((player_pos.x + PLAYER_WIDTH >= tile_pos.x && player_pos.x <= tile_pos.x + TILE_WIDTH) &&
                (player_pos.y + PLAYER_HEIGHT >= tile_pos.y && player_pos.y <= tile_pos.y + TILE_HEIGHT)) {
                if (tile == NORTH)
                    return "NORTH";
                if (tile == SOUTH)
                    return "SOUTH";
                if (tile == EAST)
                    return "EAST";
                if (tile == WEST)
                    return "WEST";
            }
        }
        tile_pos.x += TILE_WIDTH;
    }
    return "";
}

std::string data_change_room(MapData &data, Maps &map, Vector2 player_pos) {
    switch (map) {
        case hub:
            return player_change_room(data.hub, player_pos);
        case fejka:
            return player_change_room(data.fejka, player_pos);
            break;
        case jeetomal:
            return player_change_room(data.jeetomal, player_pos);
            break;
        case komolexos:
            return player_change_room(data.komolexos, player_pos);
            break;
        case kuggis:
            return player_change_room(data.kuggis, player_pos);
            break;
        case smycka:
            return player_change_room(data.smycka, player_pos);
            break;
        case vinterfint:
            return player_change_room(data.vinterfint, player_pos);
            break;
        case fugoumitak:
            return player_change_room(data.fugoumitak, player_pos);
            break;
    }

    return "";
}

void Player::input_player(float dt, Maps &map, MapData &map_data, int fd) {
    if (IsKeyDown(KEY_A) && this->show_chat == false) {
        Vector2 next_pos = this->player_pos;

        next_pos.x -= speed * dt;
        if (can_move(map_data, map, next_pos))
            this->player_pos.x = next_pos.x;
    }
    if (IsKeyDown(KEY_D) && this->show_chat == false) {
        Vector2 next_pos = this->player_pos;

        next_pos.x += speed * dt;
        if (can_move(map_data, map, next_pos))
            this->player_pos.x = next_pos.x;
    }
    if (IsKeyDown(KEY_W) && this->show_chat == false) {
        Vector2 next_pos = this->player_pos;

        next_pos.y -= speed * dt;
        if (can_move(map_data, map, next_pos))
            this->player_pos.y = next_pos.y;
    }
    if (IsKeyDown(KEY_S) && this->show_chat == false) {
        Vector2 next_pos = this->player_pos;

        next_pos.y += speed * dt;
        if (can_move(map_data, map, next_pos))
            this->player_pos.y = next_pos.y;
    }
    if (IsKeyPressed(KEY_E) && this->show_chat == false) {
        std::string new_room = data_change_room(map_data, map, this->player_pos);

        if (new_room != "") {
            std::string command = "MOVE " + new_room + '\n';

            send_message(command, "", fd);
            this->player_pos = PLAYER_DEFAULT_POS;
        }
    }


    if (IsKeyPressed(KEY_ESCAPE)) {
        // Pause menu enable
    }
}
