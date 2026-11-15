#include "game_world.hpp"
#include "helper.hpp"
#include <stdbool.h>

void GameWorld::draw_hud() {
    float room_name_x = (float(SCREEN_WIDTH) / 2) - ( GetFontTextWidth(this->room.name) / 2);

    if (this->player.show_chat == true) {
        this->player.chat.draw_chat();
        this->player.chat.draw_msg();
    }
    else {
        DrawFontText(this->font, this->room.name, room_name_x, 20);
        GameWorld::draw_player_count();
        GameWorld::draw_hp();
        GameWorld::draw_minimap();
        GameWorld::draw_quests();
        GameWorld::draw_inventory();
        GameWorld::draw_log();
        GameWorld::draw_dialog();
    }
}

// Returns the index of the clicked button, -1 if none.
int GameWorld::draw_text_buttons(std::vector<std::string> names, int x, int y, int bottom) {
    Vector2 mouse = get_virtual_mouse_position();
    int clicked = -1;

    for (size_t i = 0; i < names.size(); i++) {
        int width = MeasureText(names[i].c_str(), HUD_FONT_SIZE);
        bool hovered = CheckCollisionPointRec(mouse, Rectangle{float(x), float(y), float(width), HUD_FONT_SIZE});

        DrawText(names[i].c_str(), x, y, HUD_FONT_SIZE, hovered ? YELLOW : WHITE);
        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            clicked = i;
        x += width + BUTTON_SPACING;
        if (i + 1 < names.size())
            DrawLine(x - BUTTON_SPACING / 2, y - HUD_PADDING, x - BUTTON_SPACING / 2, bottom, GRAY);
    }
    return clicked;
}
