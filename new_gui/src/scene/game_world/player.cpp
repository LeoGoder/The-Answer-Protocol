#include "player.hpp"
#include "helper.hpp"
#include "map_parse.hpp"
#include "raylib.h"
#include "server.hpp"
#include <cstddef>
#include <cstdio>
#include <iostream>
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

Player::Player(Texture2D *font, std::string name, Texture2D *sprite) {
    this->player_pos = {(float(MAP_WIDTH) / 2) * TILE_WIDTH, (float(MAP_HEIGHT) / 2) * TILE_HEIGHT};
    this->font = font;
    this->name = name;
    this->sprite = sprite;
}

Player::Player() {

}

Player::~Player() {

}

void Player::draw_player(std::string player_name) {
    float player_center = (float(this->sprite->width) / 2.0f);
    float text_len_full = float(GetFontTextWidth(player_name));
    float text_len = float(GetFontTextWidth(player_name) / 2.0f - (float(CHAR_WIDTH) / 2));

    DrawRectangle(this->player_pos.x - text_len + player_center - (float(CHAR_WIDTH) / 4), this->player_pos.y - 20 - (float(CHAR_WIDTH) / 6), text_len_full, CHAR_HEIGHT, ColorAlpha(BLACK, 0.6));
    DrawFontText(*this->font, player_name, this->player_pos.x - text_len + player_center, this->player_pos.y - 20);
    DrawTexture(*this->sprite, this->player_pos.x, this->player_pos.y, WHITE);
}

bool player_overlap(MapData &data, Vector2 player_pos) {
    if (data.hub.empty()) return false;

    int max_x = TILE_WIDTH * MAP_WIDTH;
    size_t layer = data.hub.size() - 2;
    Vector2 tile_pos = {0.0, 0.0};

    for (size_t y = 0; y < data.hub[layer].size(); y++) {
        if (tile_pos.x >= max_x) {
            tile_pos.x = 0.0;
            tile_pos.y += TILE_HEIGHT;
        }
        if (data.hub[layer][y] != 0) {
            if ((player_pos.x + PLAYER_WIDTH >= tile_pos.x && player_pos.x <= tile_pos.x + TILE_WIDTH) &&
                (player_pos.y + PLAYER_HEIGHT >= tile_pos.y && player_pos.y <= tile_pos.y + TILE_HEIGHT)) {
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
            return !player_overlap(data, player_pos);
        case fejka:
            break;
        case jeetomal:
            break;
        case komolexos:
            break;
        case kuggis:
            break;
        case smycka:
            break;
        case vinterfint:
            break;
        case fugoumitak:
            break;
    }

    return false;
}

std::string player_change_room(MapData &data, Vector2 player_pos) {
    if (data.hub.empty()) return "";

    int max_x = TILE_WIDTH * MAP_WIDTH;
    size_t layer = data.hub.size() - 1;
    Vector2 tile_pos = {0.0, 0.0};

    for (size_t y = 0; y < data.hub[layer].size(); y++) {
        int tile = data.hub[layer][y];

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
                if (tile == WEAST)
                    return "WEAST";
            }
        }
        tile_pos.x += TILE_WIDTH;
    }
    return "";
}

void Player::input_player(float dt, Maps &map, MapData &map_data, int fd) {
    if (IsKeyDown(KEY_A)) {
        Vector2 next_pos = this->player_pos;

        next_pos.x -= speed * dt;
        if (can_move(map_data, map, next_pos))
            this->player_pos.x = next_pos.x;
    }
    if (IsKeyDown(KEY_D)) {
        Vector2 next_pos = this->player_pos;

        next_pos.x += speed * dt;
        if (can_move(map_data, map, next_pos))
            this->player_pos.x = next_pos.x;
    }
    if (IsKeyDown(KEY_W)) {
        Vector2 next_pos = this->player_pos;

        next_pos.y -= speed * dt;
        if (can_move(map_data, map, next_pos))
            this->player_pos.y = next_pos.y;
    }
    if (IsKeyDown(KEY_S)) {
        Vector2 next_pos = this->player_pos;

        next_pos.y += speed * dt;
        if (can_move(map_data, map, next_pos))
            this->player_pos.y = next_pos.y;
    }
    if (IsKeyPressed(KEY_E)) {
        std::string new_room = player_change_room(map_data, this->player_pos);

        if (new_room != "") {
            std::string command = "MOVE " + new_room + '\n';

            send_message(command, "", fd);
        }
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        // Pause menu enable
    }
}
