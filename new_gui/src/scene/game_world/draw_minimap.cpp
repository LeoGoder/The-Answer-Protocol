#include <map>
#include <string>

#include "inc/game_world.hpp"
#include "inc/helper.hpp"
#include "inc/raylib.h"

// Grid cell of each room, laid out from the exits of world.json.
static std::map<std::string, Vector2> room_cells = {
    {"fejka", {0, 0}}, {"smycka", {1, 0}},
    {"vinterfint", {0, 1}}, {"jeetomal", {1, 1}}, {"kuggis", {2, 1}},
    {"komolexos", {0, 2}}, {"hub", {1, 2}}, {"fugoumitak", {2, 2}},
};

static const char *room_links[][2] = {
    {"hub", "komolexos"},       {"hub", "jeetomal"},
    {"hub", "fugoumitak"},      {"komolexos", "vinterfint"},
    {"vinterfint", "jeetomal"}, {"vinterfint", "fejka"},
    {"jeetomal", "smycka"},     {"jeetomal", "kuggis"},
    {"smycka", "fejka"},        {"smycka", "kuggis"},
};

static Vector2 cell_center(const std::string &room) {
    Vector2 cell = room_cells[room];
    float x = SCREEN_WIDTH - 10 - MINIMAP_SIZE + HUD_PADDING + MINIMAP_CELL / 2;
    float y = 10 + HUD_PADDING + MINIMAP_CELL / 2;

    return Vector2{x + cell.x * MINIMAP_STEP, y + cell.y * MINIMAP_STEP};
}

void GameWorld::draw_minimap() {
    DrawRectangle(SCREEN_WIDTH - 10 - MINIMAP_SIZE, 10, MINIMAP_SIZE,
                  MINIMAP_HEIGHT, ColorAlpha(BLACK, 0.6));
    GameWorld::draw_player_count(SCREEN_WIDTH - 10 - MINIMAP_SIZE,
                                 10 + MINIMAP_SIZE - HUD_PADDING, MINIMAP_SIZE);
    for (size_t i = 0; i < sizeof(room_links) / sizeof(room_links[0]); i++)
        DrawLineV(cell_center(room_links[i][0]), cell_center(room_links[i][1]),
                  GRAY);
    for (std::map<std::string, Vector2>::iterator it = room_cells.begin();
         it != room_cells.end(); it++) {
        Vector2 center = cell_center(it->first);

        DrawRectangle(center.x - MINIMAP_CELL / 2, center.y - MINIMAP_CELL / 2,
                      MINIMAP_CELL, MINIMAP_CELL,
                      it->first == this->room.id ? YELLOW : GRAY);
    }
}

void GameWorld::draw_quests() {
    int x = SCREEN_WIDTH - 10 - QUESTS_WIDTH;
    int y = 10 + MINIMAP_HEIGHT + 4;
    int height = (this->quests.size() + 1) * LOG_LINE_HEIGHT + HUD_PADDING * 2;

    DrawRectangle(x, y, QUESTS_WIDTH, height, ColorAlpha(BLACK, 0.6));
    DrawText("Quests", x + HUD_PADDING, y + HUD_PADDING, HUD_FONT_SIZE, YELLOW);
    for (size_t i = 0; i < this->quests.size(); i++)
        DrawText(this->quests[i].c_str(), x + HUD_PADDING,
                 y + HUD_PADDING + (i + 1) * LOG_LINE_HEIGHT, HUD_FONT_SIZE,
                 WHITE);
}
