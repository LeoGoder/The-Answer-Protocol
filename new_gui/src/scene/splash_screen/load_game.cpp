#include "splash_screen.hpp"
#include "helper.hpp"

void SplashScreen::draw_load_game() {
    float center_x =
        (SCREEN_WIDTH / 2.0f) - (this->btn_save[0]->get_sprite_width() / 2.0f);
    float sprite_height = this->btn_save[0]->get_sprite_height();
    int pos_x = -15;

    this->btn_save[0]->draw_button(font, center_x, pos_x, 1.0f);
    this->btn_save[1]->draw_button(font, center_x, pos_x + sprite_height, 1.0f);
    this->btn_save[2]->draw_button(font, center_x, pos_x + (sprite_height * 2),
                                   1.0f);
    this->btn_save[3]->draw_button(font, center_x, pos_x + (sprite_height * 3),
                                   1.0f);
    // X right of each used slot, at the height of its name.
    for (int i = 0; i <= 3; i++) {
        if (is_slot_used(i)) {
            float x = center_x + this->btn_save[i]->get_sprite_width() + 10;
            float y = pos_x + (sprite_height * i) +
                      (sprite_height - CHAR_HEIGHT) / 2.0f;

            this->btn_delete[i]->draw_button(font, x, y, 1.0f);
        }
    }
    input_hint(this->font, this->mouse_keyboard_icons, "back");
}
