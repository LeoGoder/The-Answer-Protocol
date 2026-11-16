#include "game_world.hpp"
#include "helper.hpp"
#include <stdbool.h>

void GameWorld::draw_hud() {
    float room_name_x = (float(SCREEN_WIDTH) / 2) - ( GetFontTextWidth(this->room.name) / 2);

    DrawFontText(this->font, this->room.name, room_name_x, 20);
    GameWorld::draw_player_info();
    GameWorld::draw_minimap();
    GameWorld::draw_quests();
    GameWorld::draw_inventory();
    GameWorld::draw_log();
    GameWorld::draw_dialog();
}

// Small yellow speech bubble at the top right corner of a button with unread lines.
void GameWorld::draw_bubble(float x, float y) {
    DrawRectangleRounded(Rectangle{x - 5, y - 6, 11, 7}, 0.6, 4, YELLOW);
    DrawTriangle(Vector2{x - 3, y + 1}, Vector2{x - 2, y + 4}, Vector2{x + 1, y + 1}, YELLOW);
}

// Boxed buttons in a row; returns the index of the clicked one, -1 if none. The selected one stays yellow.
int GameWorld::draw_text_buttons(std::vector<std::string> names, int x, int y, int selected) {
    Vector2 mouse = get_virtual_mouse_position();
    int clicked = -1;

    for (size_t i = 0; i < names.size(); i++) {
        Rectangle box = {float(x), float(y), float(MeasureText(names[i].c_str(), HUD_FONT_SIZE) + BUTTON_PADDING * 2), BUTTON_HEIGHT};
        bool hovered = CheckCollisionPointRec(mouse, box);
        Color color = hovered || int(i) == selected ? YELLOW : WHITE;

        DrawRectangleRec(box, ColorAlpha(BLACK, 0.6));
        DrawRectangleLinesEx(box, 1, hovered || int(i) == selected ? YELLOW : GRAY);
        DrawText(names[i].c_str(), x + BUTTON_PADDING, y + BUTTON_PADDING, HUD_FONT_SIZE, color);
        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            clicked = i;
        x += box.width + BUTTON_SPACING;
    }
    return clicked;
}
