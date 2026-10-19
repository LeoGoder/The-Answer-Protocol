#include "helper.hpp"
#include "raylib.h"

std::string GetAssetPath(const std::string &path) {
    if (FileExists(path.c_str())) return path;
    std::string res = "res/" + path;
    if (FileExists(res.c_str())) return res;
    std::string bin = "bin/" + path;
    if (FileExists(bin.c_str())) return bin;
    return path;
}

float GetFontTextWidth(const std::string &text, float scale) {
    return static_cast<float>(text.length()) * (CHAR_WIDTH * scale);
}

void DrawFontText(const Texture2D &font, const std::string &text, float x, float y, Color color, float scale) {
    float temp_x = x;
    float temp_y = y;
    float scaled_w = CHAR_WIDTH * scale;
    float scaled_h = CHAR_HEIGHT * scale;

    for (char c : text) {
        if (c == ' ') {
            temp_x += scaled_w;
            continue;
        }
        if (c == '\n') {
            temp_x = x;
            temp_y += scaled_h;
            continue;
        }

        int index = static_cast<int>(c) - FIRST_CHAR;
        if (index < 0) {
            temp_x += scaled_w;
            continue;
        }

        int col = index % CHAR_PER_ROW;
        int row = index / CHAR_PER_ROW;

        Rectangle src = { static_cast<float>(col * CHAR_WIDTH), static_cast<float>(row * CHAR_HEIGHT), (float)CHAR_WIDTH, (float)CHAR_HEIGHT };
        Rectangle dst = { temp_x, temp_y, scaled_w, scaled_h };
        DrawTexturePro(font, src, dst, Vector2{0, 0}, 0.0f, color);
        temp_x += scaled_w;
    }
}

void draw_sprite_sheet(Texture2D &spritesheet, int frame, int nb_col, Vector2 pos, Vector2 sprite_size, int scale) {
    int col = frame % nb_col;
    int row = frame / nb_col;
    Vector2 size_scaled = {sprite_size.x * scale, sprite_size.y * scale};
    Rectangle src = {col * sprite_size.x, row * sprite_size.y, sprite_size.x, sprite_size.y};
    Rectangle dst = {pos.x, pos.y, size_scaled.x, size_scaled.y};

    DrawTexturePro(spritesheet, src, dst, Vector2 {0.0, 0.0}, 0.0f, WHITE);
}

void input_hint(Texture2D &font, Texture2D &spritesheet, std::string text) {
    float text_size = GetFontTextWidth(text);
    float text_x = float(SCREEN_WIDTH - SPRITE_SIZE.x - text_size);
    float text_y = float(SCREEN_HEIGHT - SPRITE_SIZE.y - 10);

    draw_sprite_sheet(spritesheet, 1, 2, Vector2 {text_x, text_y}, SPRITE_SIZE, 1.0f);
    DrawFontText(font, text, text_x + SPRITE_SIZE.x, text_y + SPRITE_SIZE.y / 2);
}

Vector2 get_virtual_mouse_position() {
    Vector2 mouse = GetMousePosition();
    return Vector2{
        mouse.x * ((float)SCREEN_WIDTH / (float)GetScreenWidth()),
        mouse.y * ((float)SCREEN_HEIGHT / (float)GetScreenHeight())
    };
}
