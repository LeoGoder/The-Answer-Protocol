#pragma once
#include <tyra>

void load_image(Tyra::Sprite *sprite, Tyra::Engine* engine, std::string img_path);
void draw_text(Tyra::Engine *engine, const std::string text, Tyra::Sprite *font, float x, float y);
