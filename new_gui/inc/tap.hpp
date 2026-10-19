#pragma once
#include "game_state.hpp"
#include "raylib.h"

class Tap {
public:
    Texture2D mouse_cursor;
    Tap();
    ~Tap();

    void init();
    void loop();
    void draw_cursor();

private:
    GameState game_state;
};
