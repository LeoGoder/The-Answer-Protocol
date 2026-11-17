#pragma once

#include <memory>
#include <string>
#include <vector>

#include "inc/chat.hpp"
#include "inc/map_parse.hpp"
#include "inc/raylib.h"

#define NORTH 86
#define EAST 90
#define SOUTH 74
#define WEST 88

#define PLAYER_WIDTH 28
#define PLAYER_HEIGHT 28
#define PLAYER_OFFSET 16
#define PLAYER_DEFAULT_POS \
    {(float(MAP_WIDTH) / 2) * TILE_WIDTH, (float(MAP_HEIGHT) / 2) * TILE_HEIGHT}

#define PLAYER_IDLE_ANIMATION 6
#define PLAYER_WALKING_ANIMATION 4

#define TARGET_SPEED 0.1

#define IDLE_LEFT (6 * 6)
#define IDLE_RIGHT (6 * 2)
#define IDLE_BACK (6 * 4)
#define IDLE_FRONT (6 * 0)

#define WALK_LEFT (6 * 14)
#define WALK_RIGHT (6 * 10)
#define WALK_BACK (6 * 12)
#define WALK_FRONT (6 * 8)

#define WALK_LEFT_DIAG_UP (6 * 13)
#define WALK_LEFT_DIAG_DOWN (6 * 15)
#define WALK_RIGHT_DIAG_UP (6 * 11)
#define WALK_RIGHT_DIAG_DOWN (6 * 9)

class Player {
 private:
        std::vector<std::string> inventory;

 public:
        bool show_chat;
        float speed = 200.0;
        float time;
        int idle_animation;
        int walk_animation;
        int last_direction;
        Vector2 player_pos;
        Vector2 last_player_pos;

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
        void draw_other_player(std::string player_name, float dt);
        void input_player(float dt, Maps &map, MapData &map_data, int fd);
        void update_animation_timer(float dt);
};

std::string player_change_room(std::vector<std::vector<int>> &data,
                               Vector2 player_pos);

void update_player_pos(std::vector<std::unique_ptr<Player>> &vect_players,
                       std::string event);
