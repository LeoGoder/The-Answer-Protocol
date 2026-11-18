#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "inc/game_world.hpp"
#include "inc/helper.hpp"
#include "inc/map_parse.hpp"
#include "inc/player.hpp"
#include "inc/raylib.h"

std::string GameWorld::display_name(const std::string &id) {
    if (this->room.names.count(id) > 0)
        return this->room.names[id];
    return id;
}

static Vector2 slot_position(size_t index, size_t count, float row_offset) {
    Vector2 spawn = PLAYER_DEFAULT_POS;
    float row_width = (count - 1) * OBJECT_SPACING;

    return Vector2{spawn.x - row_width / 2 + index * OBJECT_SPACING,
                   spawn.y + row_offset};
}

Texture2D GameWorld::get_object_texture(const std::string &file) {
    if (this->object_textures.count(file) == 0)
        this->object_textures[file] = LoadTexture(GetAssetPath(file).c_str());
    return this->object_textures[file];
}

void GameWorld::draw_object_row(std::vector<std::string> &ids,
                                std::string prefix, float row_offset,
                                Vector2 mouse) {
    std::vector<std::string> inventory = this->player.get_inventory();

    for (size_t i = 0; i < ids.size(); i++) {
        Vector2 pos = slot_position(i, ids.size(), row_offset);
        Texture2D texture =
            GameWorld::get_object_texture(prefix + ids[i] + ".png");
        Vector2 sprite_size;
        Vector2 texture_size;
        if (prefix == "npc_") {
            sprite_size = {(static_cast<float>(texture.width) / 6),
                           (static_cast<float>(texture.height) / 8)};
            texture_size.x = sprite_size.x * 2;
            texture_size.y = sprite_size.y * 2;
        } else {
            sprite_size = {(static_cast<float>(texture.width)),
                           (static_cast<float>(texture.height))};
            texture_size.x = sprite_size.x * 2;
            texture_size.y = sprite_size.y * 2;
        }
        const int scale = 2;
        std::string name = GameWorld::display_name(ids[i]);
        bool owned = std::find(inventory.begin(), inventory.end(), ids[i]) !=
                     inventory.end();
        if (owned)
            name += " (have)";
        int name_width = MeasureText(name.c_str(), HUD_FONT_SIZE);
        float name_x = pos.x + ((sprite_size.x * scale) / 2) -
                       (static_cast<float>(name_width) / 2);
        Color name_color = WHITE;

        if (!owned && CheckCollisionPointRec(
                          mouse, Rectangle{pos.x, pos.y, texture_size.x,
                                           texture_size.y})) {
            name_color = YELLOW;
            if (prefix == "item_")
                this->hovered_item = ids[i];
            else
                this->hovered_npc = ids[i];
        }
        if (prefix == "npc_") {
            draw_sprite_sheet(texture, this->player.idle_animation, 6, pos,
                              sprite_size, scale);
            DrawRectangle(name_x - 2, pos.y - (sprite_size.y / 2) + 8,
                          name_width + 4, HUD_FONT_SIZE + 2,
                          ColorAlpha(BLACK, 0.6));
            DrawText(name.c_str(), name_x, pos.y - (sprite_size.y / 2) + 8,
                     HUD_FONT_SIZE, name_color);
        } else {
            DrawTextureEx(texture, pos, 0.0f, scale, WHITE);
            DrawRectangle(name_x - 2, pos.y + texture.height + 2,
                          name_width + 4, HUD_FONT_SIZE + 2,
                          ColorAlpha(BLACK, 0.6));
            DrawText(name.c_str(), name_x, pos.y + texture.height + 3,
                     HUD_FONT_SIZE, name_color);
        }
    }
}

// Mouse in world coordinates, {-1, -1} when it is over the inventory or the
// dialog panel.
Vector2 GameWorld::world_mouse() {
    Vector2 screen_mouse = get_virtual_mouse_position();

    if (screen_mouse.x < INVENTORY_X + SLOT_SIZE ||
        (!this->dialog_npc.empty() &&
         CheckCollisionPointRec(screen_mouse, GameWorld::dialog_rect())))
        return Vector2{-1, -1};
    return GetScreenToWorld2D(screen_mouse, this->player.cam);
}

// One label per exit, at the center of its tiles in the last map layer; yellow
// with [E] when the player stands on it.
void GameWorld::draw_exits(std::vector<std::vector<int>> &island) {
    std::vector<int> &layer = island.back();
    std::string standing = player_change_room(island, this->player.player_pos);
    std::string directions[4] = {"NORTH", "EAST", "SOUTH", "WEST"};
    int tiles[4] = {NORTH, EAST, SOUTH, WEST};
    std::string targets[4] = {this->room.exists.north, this->room.exists.east,
                              this->room.exists.south, this->room.exists.west};
    Vector2 mouse = GameWorld::world_mouse();

    this->hovered_exit.clear();
    for (int d = 0; d < 4; d++) {
        Vector2 center = {0, 0};
        int count = 0;

        for (size_t i = 0; i < layer.size(); i++) {
            if (layer[i] != tiles[d])
                continue;
            center.x += (i % MAP_WIDTH) * TILE_WIDTH + TILE_WIDTH / 2;
            center.y += (i / MAP_WIDTH) * TILE_HEIGHT + TILE_HEIGHT / 2;
            count++;
        }
        if (targets[d].empty() || count == 0)
            continue;
        std::string name = GameWorld::display_name(targets[d]);

        if (standing == directions[d])
            name += " [E]";
        int width = MeasureText(name.c_str(), HUD_FONT_SIZE);
        Rectangle label = {center.x / count - width / 2 - 2,
                           center.y / count - HUD_FONT_SIZE / 2 - 1,
                           static_cast<float>(width + 4), HUD_FONT_SIZE + 2};
        bool hovered = CheckCollisionPointRec(mouse, label);

        if (hovered)
            this->hovered_exit = directions[d];
        DrawRectangleRec(label, ColorAlpha(BLACK, 0.6));
        DrawText(name.c_str(), label.x + 2, label.y + 1, HUD_FONT_SIZE,
                 hovered || standing == directions[d] ? YELLOW : WHITE);
    }
}

void GameWorld::draw_world_objects() {
    Vector2 mouse = GameWorld::world_mouse();

    this->hovered_item.clear();
    this->hovered_npc.clear();
    GameWorld::draw_object_row(this->room.npcs, "npc_", NPC_ROW_OFFSET, mouse);
    GameWorld::draw_object_row(this->room.items, "item_", ITEM_ROW_OFFSET,
                               mouse);
}
