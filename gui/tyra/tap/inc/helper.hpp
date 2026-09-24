#pragma once
#include <tyra>

void load_image(Tyra::Sprite *sprite, Tyra::Engine* engine, std::string img_path);
void draw_text(Tyra::Engine *engine, const std::string text, Tyra::Sprite *font, float x, float y);
void draw_text_color(Tyra::Engine *engine, const std::string text, Tyra::Sprite *font, float x, float y, Tyra::Color color, float scale);
