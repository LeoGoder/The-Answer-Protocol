#pragma once

class GameScene {
    public:
        virtual ~GameScene() {}
        virtual void update() = 0;
        virtual void draw() = 0;
};
