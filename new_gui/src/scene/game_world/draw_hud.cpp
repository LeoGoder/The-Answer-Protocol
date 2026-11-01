#include "game_world.hpp"
#include <stdbool.h>

void GameWorld::draw_hud() {
    if (this->player.show_chat == true) {
        this->player.chat.draw_chat();
        this->player.chat.draw_msg();
    }
}
