#include "button.hpp"
#include "debug/debug.hpp"
#include "helper.hpp"
#include "color.hpp"

Button::Button(const std::string text, const std::string img_path, std::function<void()> callback, Tyra::Engine *engine) {
    this->text = text;
    this->img_path = img_path;
    this->engine = engine;
    this->callback = callback;
    this->is_selected = false;

    load_image(&arrow, engine, "arrow.png");
    if (img_path != "None")
        load_image(&background, engine, img_path);
    TYRA_LOG("Button assets loaded in memory");
}

Button::~Button() {
    if (img_path != "None")
        this->engine->renderer.getTextureRepository().freeBySprite(background);
    this->engine->renderer.getTextureRepository().freeBySprite(arrow);
}

void Button::draw_button(Tyra::Sprite *font, float x, float y, float scale) {
    if (img_path == "None") {
        if (is_selected == false)
            draw_text(this->engine, this->text, font, x, y, scale);
        else {
            if (this->flip_arrow_animation == false)
                this->offset += speed;
            if (this->flip_arrow_animation == true)
                this->offset -= speed;
            draw_text_color(this->engine, this->text, font, x, y, Color::Yellow, 1.0f);
            draw_sprite(engine, this->arrow, x - 16 - 10 - this->offset, y, 16.0f, 16.0f);
            if (this->offset > 30)
                this->flip_arrow_animation = true;
            if (this->offset <= 0)
                this->flip_arrow_animation = false;
        }
    }
    else {
        // TODO: replace it with futur function to draw_spritesheet
        if (this->is_selected == false)
            draw_sprite_sheet(this->engine, this->background, x, y, 256, 128, 0, 2);
        else
            draw_sprite_sheet(this->engine, this->background, x, y, 256, 128, 1, 2);

        float temp_x = x + (this->background.size.x / 2) - (static_cast<float>(get_text_len(this->text, 1.0f)) / 2);
        float temp_y = y + (this->background.size.y / 2);

        draw_text(this->engine, this->text, font, temp_x, temp_y, scale);
    }
}

void Button::on_click() {
    if (this->callback) {
        this->callback();
    }
}

void loop_button_lst(Tyra::Engine *engine, std::vector<std::unique_ptr<Button>> &button_list, int &selected_id, bool &is_left_joy_centered) {
    auto &pad = engine->pad;
    int left_joy_v = pad.getLeftJoyPad().v;
    bool analog_down = false;
    bool analog_up = false;

    button_list[selected_id]->set_is_selected(false);
    if (left_joy_v > 190 && is_left_joy_centered) {
        analog_down = true;
        is_left_joy_centered = false;
    }
    else if (left_joy_v < 60 && is_left_joy_centered) {
        analog_up = true;
        is_left_joy_centered = false;
    }
    else if (left_joy_v >= 60 && left_joy_v <=  190)
        is_left_joy_centered = true;

    if (pad.getClicked().Cross) {
        button_list[selected_id]->on_click();
        selected_id = 0;
    }
    if (pad.getClicked().DpadDown || analog_down) {
        selected_id += 1;
        if (selected_id > button_list.size() - 1)
            selected_id = 0;
    }
    if (pad.getClicked().DpadUp || analog_up) {
        selected_id -= 1;
        if (selected_id < 0)
            selected_id = button_list.size() - 1;
    }
    button_list[selected_id]->set_is_selected(true);
}
