#include "game_world.hpp"
#include "helper.hpp"

void GameWorld::draw_connection_error() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(RED, 0.30));
    DrawFontText(this->font, "Connection issue !", 20, 20, WHITE, 1.0f);
    DrawFontText(this->font, this->error_log, 20, 50, WHITE, 1.0f);
    DrawFontText(this->font, "Press ESC to return", 20, 80, Color{180, 180, 180, 255}, 1.0f);
}
