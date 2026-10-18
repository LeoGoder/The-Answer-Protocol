#pragma once
#include <memory>
#include <vector>
#include <string>
#include "raylib.h"
#include "game_scene.hpp"
#include "button.hpp"
#include "game_state.hpp"
#include "save_load.hpp"

enum State {
    welcome = 0,
    main_menu = 1,
    new_game = 2,
    create = 3,
    load_game = 4,
};

class SplashScreen : public GameScene {
private:
    Texture2D font;
    Texture2D mn_background;
    Texture2D arrow;
    Texture2D btn_background;
    Texture2D btn_background_variant;

    Color font_color;
    bool color_sens;
    State current_state;

    std::vector<std::unique_ptr<Button>> button_list;
    std::vector<std::unique_ptr<Button>> btn_save;
    std::vector<std::unique_ptr<GameData>> save_lst;
    int selected_id;
    int save_slot_choice;

public:
    GameState *game_state;

    SplashScreen(GameState *game_state);
    ~SplashScreen();

    void update() override;
    void draw() override;
    void draw_welcome_screen();
    void draw_main_menu();
    void draw_new_game();
    void draw_create_player();
    void draw_load_game();
    void keyboard_input();
};
