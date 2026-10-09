#include "splash_screen.hpp"


void SplashScreen::draw_new_game() {
    this->btn_save[0]->draw_button(&font, (this->engine->renderer.core.getSettings().getWidth() / 2) - (this->btn_save[0]->get_sprite_width() / 2), 10, 1.0f);
    this->btn_save[1]->draw_button(&font, (this->engine->renderer.core.getSettings().getWidth() / 2) - (this->btn_save[0]->get_sprite_width() / 2), 110, 1.0f);
    this->btn_save[2]->draw_button(&font, (this->engine->renderer.core.getSettings().getWidth() / 2) - (this->btn_save[0]->get_sprite_width() / 2), 210, 1.0f);
    this->btn_save[3]->draw_button(&font, (this->engine->renderer.core.getSettings().getWidth() / 2) - (this->btn_save[0]->get_sprite_width() / 2), 310, 1.0f);
}

void SplashScreen::draw_create_player() {
    float pos_x = 20;
    float pos_y = engine->renderer.core.getSettings().getHeight() / 2;
    float default_pos_x = pos_x;
    float offset = 40;

    for (int i = 0; i < this->btn_keyboard.size(); i++) {
        this->btn_keyboard[i]->draw_button(&this->font, pos_x, pos_y, 2.0f);
        pos_x += offset;
        if (pos_x >= (default_pos_x + (offset * 12))) {
            // pos_y = engine->renderer.core.getSettings().getHeight() / 2;
            pos_x = 20;
            pos_y += offset;
        }
    }
}

void put_letter_name_in_player_name() {

}
