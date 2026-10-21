#include "game_world.hpp"
#include "helper.hpp"

void GameWorld::draw_island_1() {
    DrawFontText(this->font, "Connection working !", 20, 20, WHITE, 1.0f);
    DrawFontText(this->font, "Press ESC to return", 20, 50, Color{180, 180, 180, 255}, 1.0f);
    // GameWorld::server_alive();
}
