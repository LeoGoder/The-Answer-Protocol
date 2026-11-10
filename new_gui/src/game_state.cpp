#include "game_state.hpp"

GameState::GameState() {
    actual_scene = splash_screen;
    next_scene = splash_screen;
    init_scene = true;
    game_scene = nullptr;
    dt = 0.0f;
}

GameState::~GameState() {
    if (game_scene != nullptr) {
        delete game_scene;
        game_scene = nullptr;
    }
}
