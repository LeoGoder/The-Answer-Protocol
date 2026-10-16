#pragma once
#include <functional>
#include <string>
#include "raylib.h"
#include "helper.hpp"

class Button {
private:
    std::string text;
    bool is_selected;
    std::function<void()> callback;
    const Texture2D *background;
    const Texture2D *arrow;
    bool flip_arrow_animation;
    float offset;
    float speed;
    Rectangle bounds;

public:
    Button(const Texture2D *arrow, const Texture2D *background, const std::string &text, std::function<void()> callback);
    ~Button();

    void draw_button(const Texture2D &font, float x, float y, float scale = 1.0f);
    void on_click();

    float get_size_text(float scale = 1.0f) const {
        return GetFontTextWidth(this->text, scale);
    }
    float get_sprite_width() const {
        if (!this->background) return 0.0f;
        return static_cast<float>(this->background->width) / 2.0f;
    }
    float get_sprite_height() const {
        if (!this->background) return 0.0f;
        return static_cast<float>(this->background->height);
    }
    void set_is_selected(bool state) {
        this->is_selected = state;
    }
    bool get_is_selected() const {
        return this->is_selected;
    }
    Rectangle get_bounds() const {
        return this->bounds;
    }
};
