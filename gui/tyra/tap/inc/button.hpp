#pragma once
#include <functional>
#include <string>
#include <tyra>
#include "helper.hpp"

class Button {
    private:
        std::string text;
        std::string img_path;
        bool is_selected;
        std::function<void()> callback;
        Tyra::Sprite sprite;
        Tyra::Engine *engine;
    public:
        Button(const std::string text, const std::string img_path, std::function<void()> callback, Tyra::Engine *engine);
        ~Button();
        void draw_button(Tyra::Sprite *font, float x, float y, float scale);
        void on_click();
        float get_size_text(float scale) {
            return get_text_len(this->text, scale);
        }
        int get_sprite_width() {
            return this->sprite.size.x;
        }
        int get_sprite_height() {
            return this->sprite.size.y;
        }
        Tyra::Sprite get_sprite() {
            return this->sprite;
        }
        std::string get_img_path() {
            return this->img_path;
        }
        void set_is_selected(bool state) {
            this->is_selected = state;
        }
};
