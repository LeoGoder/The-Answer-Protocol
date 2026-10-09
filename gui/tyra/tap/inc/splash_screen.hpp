#pragma once
#include <memory>
#include <tyra>
#include <string>
#include "game_scene.hpp"
#include "button.hpp"
#include "save_load.hpp""

enum State {
    welcome = 0,
    main_menu = 1,
    new_game = 2,
    create = 3,
    load_game = 4,
};

class SplashScreen : public GameScene {
    private:
        std::string path_background = Tyra::FileUtils::fromCwd("oui.png");
        Tyra::Sprite background;
        Tyra::Sprite mn_background;
        Tyra::Sprite font;
        Tyra::Sprite arrow;
        Tyra::Sprite btn_background;
        Tyra::Sprite keyboard;
        Tyra::Engine *engine;
        Tyra::Color font_color;
        bool color_sens = false;
        bool is_left_joy_centered = false;
        State current_state = welcome;
        std::vector<std::unique_ptr<Button>> button_list;
        std::vector<std::unique_ptr<Button>> btn_save;
        std::vector<std::unique_ptr<Button>> btn_keyboard;
        std::vector<std::unique_ptr<GameData>> save_lst;
        int selected_id;
        int save_slot_choice;
    public:
        SplashScreen(Tyra::Engine* engine);
        ~SplashScreen();
        void update() override;
        void draw() override;
        void load_background();
        void draw_welcome_screen();
        void draw_main_menu();
        void draw_new_game();
        void draw_create_player();
        void draw_load_game();
};
