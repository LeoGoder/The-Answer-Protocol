#pragma once
#include "game_state.hpp"

class Tap {
public:
    Tap();
    ~Tap();

    void init();
    void loop();

private:
    GameState game_state;
};
