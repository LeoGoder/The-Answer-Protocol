#include "button.hpp"
#include "helper.hpp"
#include <cstdio>

Button::Button(const Texture2D *arrow, const Texture2D *background, const std::string &text, std::function<void()> callback) {
    this->arrow = arrow;
    this->background = background;
    this->text = text;
    this->callback = callback;
    this->is_selected = false;
    this->flip_arrow_animation = false;
    this->offset = 0.0f;
    this->speed = 1.5f;
    this->bounds = Rectangle{0, 0, 0, 0};
}

Button::~Button() {}

void Button::on_click() {
    if (this->callback) {
        this->callback();
    }
}

void Button::draw_button(const Texture2D &font, float x, float y, float scale) {
    Vector2 mouse_pos = get_virtual_mouse_position();

    if (this->background == nullptr) {
        float text_w = get_size_text(scale);
        float text_h = CHAR_HEIGHT * scale;
        this->bounds = Rectangle{ x - 20, y, text_w + 40, text_h };

        this->is_selected = CheckCollisionPointRec(mouse_pos, this->bounds);
        if (this->is_selected && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            this->on_click();
        }

        if (!this->is_selected) {
            DrawFontText(font, this->text, x, y, WHITE, scale);
        } else {
            if (!this->flip_arrow_animation)
                this->offset += speed;
            else
                this->offset -= speed;

            DrawFontText(font, this->text, x, y, YELLOW, 1.0f);
            if (this->arrow != nullptr) {
                Rectangle src = { 0, 0, (float)this->arrow->width, (float)this->arrow->height };
                Rectangle dst = { x - 16 - 10 - this->offset, y, 16.0f, 16.0f };
                DrawTexturePro(*this->arrow, src, dst, Vector2{0, 0}, 0.0f, WHITE);
            }
            if (this->offset > 30)
                this->flip_arrow_animation = true;
            if (this->offset <= 0)
                this->flip_arrow_animation = false;
        }
    } else {
        float btn_w = static_cast<float>(this->background->width) / 2.0f;
        float btn_h = static_cast<float>(this->background->height);
        this->bounds = Rectangle{ x, y, btn_w * scale, btn_h * scale };

        this->is_selected = CheckCollisionPointRec(mouse_pos, this->bounds);
        if (this->is_selected && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            this->on_click();
        }

        int frame = this->is_selected ? 1 : 0;
        Rectangle src = { static_cast<float>(frame) * btn_w, 0.0f, btn_w, btn_h };
        Rectangle dst = { x, y, btn_w * scale, btn_h * scale };
        DrawTexturePro(*this->background, src, dst, Vector2{0, 0}, 0.0f, WHITE);

        float temp_x;
        float temp_y;
        if (this->text.length() == 1) {
            const float glyph_w = 5.0f;
            const float glyph_h = 8.0f;
            temp_x = x + (((btn_w * scale) / 2.0f) - ((glyph_w * scale) / 2.0f));
            temp_y = y + (((btn_h * scale) / 2.0f) - ((glyph_h * scale) / 2.0f)) - (1.0f * scale);
        } else {
            temp_x = x + (((btn_w * scale) / 2.0f) - (get_size_text(scale) / 2.0f));
            temp_y = y + ((btn_h * scale) / 2.0f) - ((CHAR_HEIGHT * scale) / 2.0f);
        }
        DrawFontText(font, this->text, temp_x, temp_y, WHITE, scale);
    }
}
