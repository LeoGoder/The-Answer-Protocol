#include <tyra>


void load_image(Tyra::Sprite *sprite, Tyra::Engine* engine, std::string img_path) {
    auto &renderer = engine->renderer;
    auto &textureRepository = renderer.getTextureRepository();
    auto filepath = Tyra::FileUtils::fromCwd(img_path);
    auto *texture = textureRepository.add(filepath);
    texture->addLink(sprite->id);
}

void draw_text(Tyra::Engine *engine, const std::string text, Tyra::Sprite *font, float x, float y) {
    const int char_width = 16;
    const int char_height = 16;
    const int char_per_row = 16;
    const int first_char = 32;
    float temp_x = x;
    float temp_y = y;

    font->mode = Tyra::SpriteMode::MODE_REPEAT;
    font->size.set(char_width, char_height);
    for (char c : text) {
        if (c == ' ') {
            temp_x += char_width;
            continue;
        }
        if (c == '\n') {
            temp_x = x;
            temp_y += char_height;
            continue;
        }
        int ascii_value = static_cast<int>(c);
        int index = ascii_value - first_char;
        int col = index % char_per_row;
        int row = index / char_per_row;

        float offset_x = static_cast<float>(col * char_width);
        float offset_y = static_cast<float>(row * char_height);

        font->offset.set(offset_x, offset_y);
        font->position.set(temp_x, temp_y);
        engine->renderer.renderer2D.render(font);
        temp_x += char_width;
    }
}

