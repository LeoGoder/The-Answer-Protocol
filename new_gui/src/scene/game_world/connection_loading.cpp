#include "game_world.hpp"
#include "helper.hpp"

void GameWorld::draw_loading_screen() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(GREEN, 0.30));
    DrawFontText(this->font, "Connecting...", 20, 20, WHITE, 1.0f);
}
