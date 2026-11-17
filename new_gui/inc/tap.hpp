#pragma once

#include "inc/game_state.hpp"
#include "inc/raylib.h"

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
