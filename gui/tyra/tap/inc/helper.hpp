#pragma once
#include <tyra>
#define CHAR_WIDTH 16
#define CHAR_HEIGHT 16
#define CHAR_PER_ROW 16
#define FIRST_CHAR 32


void load_image(Tyra::Sprite *sprite, Tyra::Engine* engine, std::string img_path);
void draw_text(Tyra::Engine *engine, const std::string text, Tyra::Sprite *font, float x, float y, float scale);
void draw_text_color(Tyra::Engine *engine, const std::string text, Tyra::Sprite *font, float x, float y, Tyra::Color color, float scale);
int get_text_len(const std::string text, float scale);
void draw_sprite(Tyra::Engine *engine, Tyra::Sprite &sprite, float x, float y, float width, float height);
