#include "helper.hpp"
#include "raylib.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

std::string GetAssetPath(const std::string &path) {
    if (FileExists(path.c_str())) return path;
    std::string res = "res/" + path;
    if (FileExists(res.c_str())) return res;
    std::string bin = "bin/" + path;
    if (FileExists(bin.c_str())) return bin;
    return path;
}

void kill_return_line(std::string &str) {
    for (long unsigned int i = 0; i < str.size(); i++) {
        if (str[i] == '\n') {
            str[i] = ' ';
        }
    }
}

std::vector<std::string> split(std::string &str, char limiter, int nb_split) {
    std::vector<std::string> tokens;
    size_t start = 0;

    while (nb_split != 0) {
        size_t end = str.find(limiter, start);
        if (end == std::string::npos) {
            break;
        }
        std::string sub_str = str.substr(start, end - start);

        tokens.push_back(sub_str);
        start = end + 1;
        nb_split--;
    }
    if (start <= str.size()) {
        tokens.push_back(str.substr(start));
    }
    return tokens;
}

std::string extract_value(const std::string &src, std::string key) {
    std::string pattern = "\"" + key + "\":\"";
    size_t start = src.find(pattern);

    if (start == std::string::npos) return "";
    start += pattern.length();

    size_t end = src.find("\"", start);

    if (end == std::string::npos) return "";

    return src.substr(start, end - start);
}

std::string extract_sub_tab(const std::string &src, std::string key) {
    std::string pattern = "\"" + key + "\":{";
    size_t start = src.find(pattern);

    if (start == std::string::npos) return "";
    start += pattern.length();

    size_t end = src.find("}", start);

    if (end == std::string::npos) return "";

    return src.substr(start, end - start);
}

std::string extract_position(const std::string &src, std::string player_name) {
    std::string new_pos;
    std::string pattern = "\"" + player_name + "\":[";
    size_t start = src.find(pattern);

    if (start == std::string::npos) return "";
    start += pattern.length();

    size_t end = src.find("]", start);
    if (start == std::string::npos) return "";

    return src.substr(start, end - start);
}

std::vector<std::string> extract_vector(const std::string &src, std::string key) {
    std::vector<std::string> vector_r;
    std::string pattern = "\"" + key + "\":[";
    size_t start = src.find(pattern);

    if (start == std::string::npos) return vector_r;
    start += pattern.length();

    size_t end = src.find("]", start);
    if (end == std::string::npos) return vector_r;

    std::string element = src.substr(start, end - start);
    std::cout << element + '\n';
    size_t cursor = 0;
    while (cursor < element.size()) {
        size_t v_start = element.find('"', cursor);
        if (v_start == std::string::npos) break;

        size_t v_end = element.find('"', v_start + 1);
        if (v_end == std::string::npos) break;

        vector_r.push_back(element.substr(v_start + 1, v_end - v_start - 1));
        cursor = v_end + 1;
    }
    return vector_r;
}

std::vector<std::vector<int>> extract_layers(const std::string &src) {
    std::vector<std::vector<int>> all_layers;
        std::size_t pos = 0;

        while (true) {
            std::size_t pos_data = src.find("\"data\"", pos);
            if (pos_data == std::string::npos) {
                break;
            }

            std::size_t start = src.find('[', pos_data);
            if (start == std::string::npos) break;
            start++;

            std::size_t end = src.find(']', start);
            if (end == std::string::npos) break;

            std::string data = src.substr(start, end - start);
            std::stringstream ss(data);
            std::vector<int> current_layer;

            int id = 0;
            char comma = 0;
            while (ss >> id) {
                current_layer.push_back(id);
                ss >> comma;
            }
            all_layers.push_back(current_layer);
            pos = end + 1;
        }
        return all_layers;
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
