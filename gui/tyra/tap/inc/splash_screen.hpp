#pragma once
#include <memory>
#include <tyra>
#include <string>
#include "game_scene.hpp"
#include "button.hpp"

enum State {
    welcome = 0,
    main_menu = 1,
};

class SplashScreen : public GameScene {
    private:
        std::string path_background = Tyra::FileUtils::fromCwd("oui.png"); 
        Tyra::Sprite background;
        Tyra::Sprite mn_background;
        Tyra::Sprite font;
        Tyra::Engine *engine;
        Tyra::Color font_color;
        bool color_sens = false;
        bool is_left_joy_centered = false;
        State current_state = welcome;
        std::vector<std::unique_ptr<Button>> button_list;
        int selected_id;
    public:
        SplashScreen(Tyra::Engine* engine);
        ~SplashScreen();
        void update() override;
        void draw() override;
        void load_background();
        void draw_welcome_screen();
        void draw_main_menu();
};
