#include "button.hpp"
#include "helper.hpp"

Button::Button(const std::string text, const std::string img_path, std::function<void()> callback, Tyra::Engine *engine) {
    this->text = text;
    this->img_path = img_path;
    this->engine = engine;
    this->callback = callback;
    if (img_path != "None")
        load_image(&sprite, engine, img_path);
}

Button::~Button() {
    if (img_path != "None")
        this->engine->renderer.getTextureRepository().freeBySprite(sprite);
}

void Button::draw_button(Tyra::Sprite *font, float x, float y, float scale) {
    if (img_path == "None") {
        draw_text(this->engine, this->text, font, x, y, scale);
    }
    else {
        // TODO: replace it with futur function to draw_spritesheet
        this->engine->renderer.renderer2D.render(this->sprite);
        float temp_x = x + (this->sprite.size.x / 2) - (static_cast<float>(get_text_len(this->text, 1.0f)) / 2);
        float temp_y = y + (this->sprite.size.y / 2);
        draw_text(this->engine, this->text, font, temp_x, temp_y, scale);
    }
}

void Button::on_click() {
    if (this->callback) {
        this->callback();
    }
}
