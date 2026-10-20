#pragma once
#include <string>
#include <vector>
#include "raylib.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define CHAR_WIDTH 16
#define CHAR_HEIGHT 16
#define CHAR_PER_ROW 16
#define FIRST_CHAR 32

#define SPRITE_SIZE Vector2 {32.0, 32.0}

std::string GetAssetPath(const std::string &path);
std::vector<std::string> split(std::string &str, char limiter);
void DrawFontText(const Texture2D &font, const std::string &text, float x, float y, Color color = WHITE, float scale = 1.0f);
float GetFontTextWidth(const std::string &text, float scale = 1.0f);
Vector2 get_virtual_mouse_position();
void draw_sprite_sheet(Texture2D &spritesheet, int frame, int nb_col, Vector2 pos, Vector2 sprite_size, int scale);
void input_hint(Texture2D &font, Texture2D &spritesheet, std::string text);
