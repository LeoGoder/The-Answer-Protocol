#include "engine.hpp"
#include "math/vec2.hpp"
#include "renderer/core/2d/sprite/sprite.hpp"
#include "renderer/models/color.hpp"
#include "helper.hpp"
#include <tyra>


void load_image(Tyra::Sprite *sprite, Tyra::Engine* engine, std::string img_path) {
    auto &renderer = engine->renderer;
    auto &textureRepository = renderer.getTextureRepository();
    auto filepath = Tyra::FileUtils::fromCwd(img_path);
    auto *texture = textureRepository.add(filepath);
    texture->addLink(sprite->id);
}

int get_text_len(const std::string text, float scale) {
    int text_len = text.length() * (CHAR_WIDTH * scale);
    return text_len;
}

void draw_text(Tyra::Engine *engine, const std::string text, Tyra::Sprite *font, float x, float y, float scale) {
    float default_scale = font->scale;
    float scaled_width = CHAR_WIDTH * scale;
    float scaled_height = CHAR_HEIGHT * scale;
    float temp_x = x;
    float temp_y = y;

    font->mode = Tyra::SpriteMode::MODE_REPEAT;
    font->size.set(CHAR_WIDTH, CHAR_HEIGHT);
    font->scale = scale;
    for (char c : text) {
        if (c == ' ') {
            temp_x += scaled_width;
            continue;
        }
        if (c == '\n') {
            temp_x = x;
            temp_y += scaled_height;
            continue;
        }
        int ascii_value = static_cast<int>(c);
        int index = ascii_value - FIRST_CHAR;
        int col = index % CHAR_PER_ROW;
        int row = index / CHAR_PER_ROW;

        float offset_x = static_cast<float>(col * CHAR_WIDTH);
        float offset_y = static_cast<float>(row * CHAR_HEIGHT);

        font->offset.set(offset_x, offset_y);
        font->position.set(temp_x, temp_y);
        engine->renderer.renderer2D.render(font);
        temp_x += scaled_width;
    }
    font->scale = default_scale;
}

void draw_text_color(Tyra::Engine *engine, const std::string text, Tyra::Sprite *font, float x, float y, Tyra::Color color, float scale) {
    float temp_x = x;
    float temp_y = y;
    Tyra::Color default_color = font->color;
    float default_scale = font->scale;
    float scaled_width = CHAR_WIDTH * scale;
    float scaled_height = CHAR_HEIGHT * scale;

    font->mode = Tyra::SpriteMode::MODE_REPEAT;
    font->size.set(CHAR_WIDTH, CHAR_HEIGHT);
    font->scale = scale;
    font->color.set(color);
    for (char c : text) {
        if (c == ' ') {
            temp_x += scaled_width;
            continue;
        }
        if (c == '\n') {
            temp_x = x;
            temp_y += scaled_height;
            continue;
        }
        int ascii_value = static_cast<int>(c);
        int index = ascii_value - FIRST_CHAR;
        int col = index % CHAR_PER_ROW;
        int row = index / CHAR_PER_ROW;

        float offset_x = static_cast<float>(col * CHAR_WIDTH);
        float offset_y = static_cast<float>(row * CHAR_HEIGHT);

        font->offset.set(offset_x, offset_y);
        font->position.set(temp_x, temp_y);
        engine->renderer.renderer2D.render(font);
        temp_x += scaled_width;
    }
    font->color = default_color;
    font->scale = default_scale;
}

void draw_sprite(Tyra::Engine *engine, Tyra::Sprite &sprite, float x, float y, float width, float height) {
    sprite.mode = Tyra::SpriteMode::MODE_STRETCH;
    sprite.size = Tyra::Vec2(width, height);
    sprite.position.x = x;
    sprite.position.y = y;
    engine->renderer.renderer2D.render(sprite);
}

void draw_sprite_sheet() {

}
