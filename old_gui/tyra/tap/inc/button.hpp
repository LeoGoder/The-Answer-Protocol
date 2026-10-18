#pragma once
#include <functional>
#include <string>
#include <tyra>
#include "helper.hpp"

class Button {
    private:
        std::string text;
        bool is_selected;
        std::function<void()> callback;
        Tyra::Sprite *background = nullptr;
        Tyra::Sprite *arrow = nullptr;
        Tyra::Engine *engine = nullptr;
        bool flip_arrow_animation = false;
        float offset = 0.0f;
        float speed = 1.5f;
    public:
        // Button(Tyra::Sprite *arrow, Tyra::Sprite *background, std::function<void()> callback, Tyra::Engine *engine);
        Button(Tyra::Sprite *arrow, Tyra::Sprite *background, const std::string text, std::function<void()> callback, Tyra::Engine *engine);
        ~Button();
        void draw_button(Tyra::Sprite *font, float x, float y, float scale);
        void on_click();
        float get_size_text(float scale) {
            return get_text_len(this->text, scale);
        }
        float get_sprite_width() {
            return this->background->size.x;
        }
        float get_sprite_height() {
            return this->background->size.y;
        }
        Tyra::Sprite *get_sprite() {
            return this->background;
        }
        void set_is_selected(bool state) {
            this->is_selected = state;
        }
};

void loop_button_lst(Tyra::Engine *engine, std::vector<std::unique_ptr<Button>> &button_list, int &selected_id, bool &is_left_joy_centered, int col_number, int line_number);
