#include "splash_screen.hpp"


void SplashScreen::draw_new_game() {
    this->btn_save[0]->draw_button(&font, 100, 10, 1.0f);
    this->btn_save[1]->draw_button(&font, 100, 110, 1.0f);
    this->btn_save[2]->draw_button(&font, 100, 210, 1.0f);
    this->btn_save[3]->draw_button(&font, 100, 310, 1.0f);
}
