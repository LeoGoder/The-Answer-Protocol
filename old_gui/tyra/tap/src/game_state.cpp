#include "../inc/game_state.hpp"

GameState::GameState() {
    actual_scene = splash_screen;
    init_scene = true;
    game_scene = nullptr;
}

GameState::~GameState() {
    // nothing to do for now
}


