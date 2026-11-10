#pragma once

#include "chat.hpp"
#include "map_parse.hpp"
#include "raylib.h"
#include <memory>
#include <string>
#include <vector>

#define NORTH 86
#define EAST 90
#define SOUTH 74
#define WEST 88

#define PLAYER_WIDTH 28
#define PLAYER_HEIGHT 28
#define PLAYER_DEFAULT_POS {(float(MAP_WIDTH) / 2) * TILE_WIDTH, (float(MAP_HEIGHT) / 2) * TILE_HEIGHT}

class Player {
    private:
        std::vector<std::string> inventory;

    public:
        bool show_chat;
        float speed = 200.0;
        Vector2 player_pos;

        std::string name;

        Texture2D *sprite;
        Texture2D *font;

        Camera2D cam;
        Chat chat;

        Player(Texture2D *font, std::string name, Texture2D *sprite);
        Player();
        ~Player();

        void draw_player(std::string player_name);
        void input_player(float dt, Maps &map, MapData &map_data, int fd);
};

void update_player_pos(std::vector<std::unique_ptr<Player>> &vect_players, std::string event);
