#include <string>

#include "inc/game_world.hpp"
#include "inc/raylib.h"

// "Players <room>/<server>", centered under the minimap grid.
void GameWorld::draw_player_count(int x, int y, int width) {
    std::string count = "Players " + std::to_string(this->room.player.size()) +
                        "/" + std::to_string(this->server_players);

    DrawText(count.c_str(),
             x + width / 2 - MeasureText(count.c_str(), HUD_FONT_SIZE) / 2, y,
             HUD_FONT_SIZE, WHITE);
}

// HP and the QUIT button in one box, top left.
void GameWorld::draw_player_info() {
    std::string text =
        "HP " + std::to_string(this->hp) + "/" + std::to_string(this->max_hp);
    int text_width = MeasureText(text.c_str(), HUD_FONT_SIZE);
    int quit_width = MeasureText("QUIT", HUD_FONT_SIZE) + BUTTON_PADDING * 2;
    int height = BUTTON_HEIGHT + HUD_PADDING * 2;
    int y = INVENTORY_Y - 4 - height;

    DrawRectangle(INVENTORY_X, y, text_width + quit_width + HUD_PADDING * 3,
                  height, ColorAlpha(BLACK, 0.6));
    DrawText(text.c_str(), INVENTORY_X + HUD_PADDING,
             y + HUD_PADDING + BUTTON_PADDING, HUD_FONT_SIZE, WHITE);
    if (GameWorld::draw_text_buttons({"QUIT"},
                                     INVENTORY_X + HUD_PADDING * 2 + text_width,
                                     y + HUD_PADDING) == 0) {
        send_message(this->command.QUIT, "", this->fd);
        this->game_state->set_init_scene(true);
        this->game_state->set_actual_scene(splash_screen);
    }
}
