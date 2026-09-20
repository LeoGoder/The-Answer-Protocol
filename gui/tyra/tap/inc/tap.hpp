#pragma once

#include <tyra>
#include "game_state.hpp"

namespace Tyra {

class Tap : public Game {
    public:
        Tap(Engine* engine);
        ~Tap();

        void init();
        void loop();

    private:
        void loadTexture();
        void loadSprite();

        Engine* engine;
        Sprite sprite;
        GameState game_state;
};

}  // namespace Tyra
