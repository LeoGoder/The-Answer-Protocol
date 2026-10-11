#pragma once

#include <tyra>
#include "game_state.hpp"

namespace Tyra {

const bool IS_REAL_PS2_VIA_USB = true;

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
