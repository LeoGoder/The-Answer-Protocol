#include "splash_screen.hpp"


void SplashScreen::draw_new_game() {
    this->btn_save[0]->draw_button(&font, (this->engine->renderer.core.getSettings().getWidth() / 2) - (this->btn_save[0]->get_sprite_width() / 2), 10, 1.0f);
    this->btn_save[1]->draw_button(&font, (this->engine->renderer.core.getSettings().getWidth() / 2) - (this->btn_save[0]->get_sprite_width() / 2), 110, 1.0f);
    this->btn_save[2]->draw_button(&font, (this->engine->renderer.core.getSettings().getWidth() / 2) - (this->btn_save[0]->get_sprite_width() / 2), 210, 1.0f);
    this->btn_save[3]->draw_button(&font, (this->engine->renderer.core.getSettings().getWidth() / 2) - (this->btn_save[0]->get_sprite_width() / 2), 310, 1.0f);
}

void SplashScreen::draw_create_player() {
    
}

void put_letter_name_in_player_name() {

}
