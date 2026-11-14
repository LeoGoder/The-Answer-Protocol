#include "game_world.hpp"
#include "helper.hpp"
#include "raylib.h"
#include <cstddef>
#include <string>
#include <vector>

static std::string display_name(Room &room, const std::string &id) {
    if (room.names.count(id) > 0 && room.names[id] != "")
        return room.names[id];
    return id;
}

static Vector2 slot_position(size_t index, size_t count, float row_offset) {
    Vector2 spawn = PLAYER_DEFAULT_POS;
    float row_width = (count - 1) * OBJECT_SPACING;

    return Vector2{spawn.x - row_width / 2 + index * OBJECT_SPACING, spawn.y + row_offset};
}

Texture2D GameWorld::get_object_texture(const std::string &file) {
    if (this->object_textures.count(file) == 0)
        this->object_textures[file] = LoadTexture(GetAssetPath(file).c_str());
    return this->object_textures[file];
}

void GameWorld::draw_object_row(std::vector<std::string> &ids, std::string prefix, float row_offset, Vector2 mouse) {
    for (size_t i = 0; i < ids.size(); i++) {
        Vector2 pos = slot_position(i, ids.size(), row_offset);
        Texture2D texture = GameWorld::get_object_texture(prefix + ids[i] + ".png");
        std::string name = display_name(this->room, ids[i]);
        int name_width = MeasureText(name.c_str(), HUD_FONT_SIZE);
        float name_x = pos.x + OBJECT_SIZE / 2 - name_width / 2;
        Color name_color = WHITE;

        if (CheckCollisionPointRec(mouse, Rectangle{pos.x, pos.y, OBJECT_SIZE, OBJECT_SIZE}))
            name_color = YELLOW;
        DrawTexture(texture, pos.x, pos.y, WHITE);
        DrawRectangle(name_x - 2, pos.y + OBJECT_SIZE + 2, name_width + 4, HUD_FONT_SIZE + 2, ColorAlpha(BLACK, 0.6));
        DrawText(name.c_str(), name_x, pos.y + OBJECT_SIZE + 3, HUD_FONT_SIZE, name_color);
    }
}

void GameWorld::draw_world_objects() {
    Vector2 mouse = GetScreenToWorld2D(get_virtual_mouse_position(), this->player.cam);

    GameWorld::draw_object_row(this->room.npcs, "npc_", NPC_ROW_OFFSET, mouse);
    GameWorld::draw_object_row(this->room.items, "item_", ITEM_ROW_OFFSET, mouse);
}
