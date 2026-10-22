#pragma once
#include "raylib.h"
#include <string>
#include <vector>

class Player {
    private:
        std::vector<std::string> inventory;

    public:
        float speed = 100.0;
        Vector2 player_pos;
        std::string name;
        Texture2D *sprite;
        Texture2D *font;

        Player(Texture2D *font, std::string name, Texture2D *sprite);
        Player();
        ~Player();

        void draw_player(std::string player_name);
        void input_player(float dt);
};