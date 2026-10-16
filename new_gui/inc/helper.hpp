#pragma once
#include <string>
#include "raylib.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define CHAR_WIDTH 16
#define CHAR_HEIGHT 16
#define CHAR_PER_ROW 16
#define FIRST_CHAR 32

std::string GetAssetPath(const std::string &path);
void DrawFontText(const Texture2D &font, const std::string &text, float x, float y, Color color = WHITE, float scale = 1.0f);
float GetFontTextWidth(const std::string &text, float scale = 1.0f);
