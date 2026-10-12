#pragma once
#include "game_scene.hpp"

class GameWorld : public GameScene {
    private:
        int temp;
    public:
        GameWorld();
        ~GameWorld();
        void update() override;
        void draw() override;
};
