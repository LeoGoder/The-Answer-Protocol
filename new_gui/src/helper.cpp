#include "helper.hpp"

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

Vector2 get_virtual_mouse_position() {
    Vector2 mouse = GetMousePosition();
    return Vector2{
        mouse.x * ((float)SCREEN_WIDTH / (float)GetScreenWidth()),
        mouse.y * ((float)SCREEN_HEIGHT / (float)GetScreenHeight())
    };
}
