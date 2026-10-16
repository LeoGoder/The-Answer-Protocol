#include "splash_screen.hpp"
#include "helper.hpp"

void SplashScreen::draw_new_game() {
    float center_x = (SCREEN_WIDTH / 2.0f) - (this->btn_save[0]->get_sprite_width() / 2.0f);
    this->btn_save[0]->draw_button(font, center_x, 10, 1.0f);
    this->btn_save[1]->draw_button(font, center_x, 110, 1.0f);
    this->btn_save[2]->draw_button(font, center_x, 210, 1.0f);
    this->btn_save[3]->draw_button(font, center_x, 310, 1.0f);
}

void SplashScreen::draw_create_player() {
    std::string temp_str = this->save_lst[this->save_slot_choice]->player_name;
    if (temp_str.empty()) temp_str = "Player";
    DrawFontText(this->font, temp_str, (SCREEN_WIDTH / 2.0f) - (GetFontTextWidth(temp_str, 2.0f) / 2.0f), 50, this->font_color, 2.0f);
}
