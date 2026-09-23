#pragma once
#include <tyra>
#include <string>
#include "game_scene.hpp"

class SplashScreen : public GameScene {
    private:
        std::string path_background = Tyra::FileUtils::fromCwd("oui.png"); 
        Tyra::Sprite background;
        Tyra::Sprite font;
        Tyra::Engine *engine;
    public:
        SplashScreen(Tyra::Engine* engine);
        ~SplashScreen();
        void update() override;
        void draw() override;
        void load_background();
};
