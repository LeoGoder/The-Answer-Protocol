#include "game_world.hpp"
#include "helper.hpp"
#include "raylib.h"
#include <string>

void GameWorld::draw_player_count() {
    std::string count = "Players " + std::to_string(this->room.player.size()) + "/" + std::to_string(this->server_players);
    int width = MeasureText(count.c_str(), HUD_FONT_SIZE) + HUD_PADDING * 2;
    int height = HUD_FONT_SIZE + HUD_PADDING * 2;

    DrawRectangle(10, SCREEN_HEIGHT - 10 - height, width, height, ColorAlpha(BLACK, 0.6));
    DrawText(count.c_str(), 10 + HUD_PADDING, SCREEN_HEIGHT - 10 - height + HUD_PADDING, HUD_FONT_SIZE, WHITE);
}