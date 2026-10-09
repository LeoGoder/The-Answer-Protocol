#include "button.hpp"
#include "helper.hpp"
#include "color.hpp"
#include "renderer/core/2d/sprite/sprite.hpp"

Button::Button(Tyra::Sprite *arrow, Tyra::Sprite *background, const std::string text, std::function<void()> callback, Tyra::Engine *engine) {
    this->engine = engine;
    this->callback = callback;
    this->is_selected = false;
    this->arrow = arrow;
    this->background = background;
    this->text = text;
}

Button::~Button() {

}

void Button::draw_button(Tyra::Sprite *font, float x, float y, float scale) {
    if (this->background == nullptr) {
        if (is_selected == false)
            draw_text(this->engine, this->text, font, x, y, scale);
        else {
            if (this->flip_arrow_animation == false)
                this->offset += speed;
            if (this->flip_arrow_animation == true)
                this->offset -= speed;
            draw_text_color(this->engine, this->text, font, x, y, Color::Yellow, 1.0f);
            draw_sprite(engine, *this->arrow, x - 16 - 10 - this->offset, y, 16.0f, 16.0f);
            if (this->offset > 30)
                this->flip_arrow_animation = true;
            if (this->offset <= 0)
                this->flip_arrow_animation = false;
        }
    }
    else {
        auto* texture = this->engine->renderer.core.texture.repository.getBySpriteId(this->background->id);
        float btn_w = (static_cast<float>(texture->getWidth()) / 2);
        float btn_h = static_cast<float>(texture->getHeight());
        float temp_x;
        float temp_y;

        if (this->is_selected == false)
            draw_sprite_sheet(this->engine, *this->background, x, y, btn_w, btn_h, 0, 2, scale);
        else
            draw_sprite_sheet(this->engine, *this->background, x, y, btn_w, btn_h, 1, 2, scale);
        if (this->text.length() == 1) {
            const float glyph_w = 5.0f;
            const float glyph_h = 8.0f;

            temp_x = x + (((btn_w * scale) / 2.0f) - ((glyph_w * scale) / 2.0f));
            temp_y = y + (((btn_h * scale) / 2.0f) - ((glyph_h * scale) / 2.0f)) - (1.0f * scale);
        }
        else {
            temp_x = x + (((btn_w * scale) / 2.0f) - (static_cast<float>(get_text_len(this->text, scale)) / 2.0f));
            temp_y = y + ((btn_h * scale) / 2.0f) - (static_cast<float>(CHAR_HEIGHT * scale) / 2.0f);
        }
        draw_text(this->engine, this->text, font, temp_x, temp_y, scale);
    }
}

void Button::on_click() {
    if (this->callback) {
        this->callback();
    }
}

void loop_button_lst(Tyra::Engine *engine, std::vector<std::unique_ptr<Button>> &button_list, int &selected_id, bool &is_left_joy_centered, int col_number, int line_number) {
    auto &pad = engine->pad;
    int left_joy_v = pad.getLeftJoyPad().v;
    int left_joy_h = pad.getLeftJoyPad().h;
    bool analog_down = false;
    bool analog_up = false;
    bool analog_left = false;
    bool analog_right = false;

    button_list[selected_id]->set_is_selected(false);
    if (is_left_joy_centered) {
        if (left_joy_v > 190) {
            analog_down = true;
            is_left_joy_centered = false;
        }
        else if (left_joy_v < 60) {
            analog_up = true;
            is_left_joy_centered = false;
        }
        else if (left_joy_h > 190) {
            analog_right = true;
            is_left_joy_centered = false;
        }
        else if (left_joy_h < 60) {
            analog_left = true;
            is_left_joy_centered = false;
        }
    }
    else if ((left_joy_v >= 60 && left_joy_v <= 190) && (left_joy_h >= 60 && left_joy_h <= 190)) {
        is_left_joy_centered = true;
    }

    if (pad.getClicked().Cross) {
        button_list[selected_id]->on_click();
        selected_id = 0;
    }
    if (pad.getClicked().DpadRight || analog_right) {
        if (col_number > 1) {
            selected_id += 1;
            if (selected_id > static_cast<int>(button_list.size()) - 1)
                selected_id = 0;
        }
    }
    if (pad.getClicked().DpadLeft || analog_left) {
        if (col_number > 1) {
            selected_id -= 1;
            if (selected_id < 0)
                selected_id = button_list.size() - 1;
        }
    }
    if (pad.getClicked().DpadDown || analog_down) {
        int old_id = selected_id;
        selected_id += col_number;
        if (selected_id > static_cast<int>(button_list.size()) - 1) {
            if (col_number == 1)
                selected_id = 0;
            else
                selected_id = old_id;
        }
    }
    if (pad.getClicked().DpadUp || analog_up) {
        int old_id = selected_id;
        selected_id -= col_number;
        if (selected_id < 0) {
            if (col_number == 1)
                selected_id = button_list.size() - 1;
            else
                selected_id = old_id;
        }
    }
    button_list[selected_id]->set_is_selected(true);
}
