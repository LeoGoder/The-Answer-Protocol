#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "inc/game_world.hpp"
#include "inc/helper.hpp"
#include "inc/raylib.h"

void GameWorld::draw_inventory() {
    std::vector<std::string> inventory = this->player.get_inventory();
    Vector2 mouse = get_virtual_mouse_position();
    std::string hovered;
    float hovered_y = 0;

    this->hovered_slot.clear();
    for (size_t i = 0; i < INVENTORY_SLOTS; i++) {
        Rectangle slot = {INVENTORY_X,
                          static_cast<float>(INVENTORY_Y + i * SLOT_SIZE),
                          SLOT_SIZE - 2, SLOT_SIZE - 2};

        DrawRectangleRec(slot, ColorAlpha(BLACK, 0.6));
        if (i >= inventory.size())
            continue;
        DrawTexture(
            GameWorld::get_object_texture("item_" + inventory[i] + ".png"),
            slot.x + 1, slot.y + 1, WHITE);
        if (CheckCollisionPointRec(mouse, slot)) {
            bool usable = std::find(this->usable.begin(), this->usable.end(),
                                    inventory[i]) != this->usable.end();

            hovered = GameWorld::display_name(inventory[i]) +
                      (usable ? "  L: use  R: drop" : "  R: drop");
            hovered_y = slot.y;
            this->hovered_slot = inventory[i];
        }
    }
    if (hovered.empty())
        return;
    int width = MeasureText(hovered.c_str(), HUD_FONT_SIZE) + HUD_PADDING * 2;

    DrawRectangle(INVENTORY_X + SLOT_SIZE, hovered_y, width,
                  HUD_FONT_SIZE + HUD_PADDING * 2, ColorAlpha(BLACK, 0.6));
    DrawText(hovered.c_str(), INVENTORY_X + SLOT_SIZE + HUD_PADDING,
             hovered_y + HUD_PADDING, HUD_FONT_SIZE, WHITE);
}
