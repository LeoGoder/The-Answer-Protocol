#include "game_world.hpp"
#include "helper.hpp"
#include <stdbool.h>

void GameWorld::draw_hud() {
    float room_name_x = (float(SCREEN_WIDTH) / 2) - ( GetFontTextWidth(this->room.name) / 2);

    if (this->player.show_chat == true) {
        this->player.chat.draw_chat();
        this->player.chat.draw_msg();
    }
    else
        DrawFontText(this->font, this->room.name, room_name_x, 20);
}
