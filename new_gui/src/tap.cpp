#include "tap.hpp"
#include "raylib.h"
#include "splash_screen.hpp"
#include "game_world.hpp"
#include <cstdio>

Tap::Tap() : game_state() {
    this->mouse_cursor = LoadTexture(GetAssetPath("cursor.png").c_str());
}

Tap::~Tap() {}

void Tap::init() {
    game_state.dt = 0.0f;
}

void Tap::draw_cursor() {
    Vector2 mouse_pos = get_virtual_mouse_position();

    DrawTexture(this->mouse_cursor, mouse_pos.x, mouse_pos.y, WHITE);
    if (!IsCursorOnScreen())
        ShowCursor();
    else
        HideCursor();
}

void Tap::loop() {
    GameScene *game_scene = game_state.get_game_scene();
    Scene scene = game_state.get_actual_scene();

    if (game_state.get_init_scene()) {
        if (game_scene != nullptr) {
            delete game_scene;
            game_scene = nullptr;
        }
        switch (scene) {
            case splash_screen:
                game_state.set_game_scene(new SplashScreen(&game_state));
                break;
            case game:
                game_state.set_game_scene(new GameWorld(&game_state));
                break;
            case end_screen:
                break;
            default:
                break;
        }
        game_state.set_init_scene(false);
        game_scene = game_state.get_game_scene();
    }

    if (game_scene != nullptr) {
        game_scene->update();
        game_scene->draw();
    }
    Tap::draw_cursor();
    game_state.dt = GetFrameTime();
}
