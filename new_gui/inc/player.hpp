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

#define PLAYER_IDLE_ANIMATION 6
#define PLAYER_WALKING_ANIMATION 4

#define TARGET_SPEED 0.1
#define TRANSITION (8 * 6)
#define LEFT (6 * 6)
#define RIGHT (6 * 2)
#define BACK (6 * 4)

class Player {
    private:
        std::vector<std::string> inventory;

    public:
        bool show_chat;
        float speed = 200.0;
        float time;
        int animation;
        Vector2 player_pos;

        std::string name;

        Texture2D *sprite;
        Texture2D *font;

        Camera2D cam;
        Chat chat;

        std::vector<std::string> get_inventory() const {
            return this->inventory;
        }
        void set_inventory(std::vector<std::string> new_inventory) {
            this->inventory = new_inventory;
        }

        Player(Texture2D *font, std::string name, Texture2D *sprite);
        Player();
        ~Player();

        void draw_player(std::string player_name, float dt);
        void input_player(float dt, Maps &map, MapData &map_data, int fd);
};

std::string player_change_room(std::vector<std::vector<int>> &data, Vector2 player_pos);

void update_player_pos(std::vector<std::unique_ptr<Player>> &vect_players, std::string event);
