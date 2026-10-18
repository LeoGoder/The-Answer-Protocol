#include "raylib.h"
#include "splash_screen.hpp"
#include "helper.hpp"
#include <cctype>
#include <cstdio>
#include <cstring>

void SplashScreen::draw_new_game() {
    float center_x = (SCREEN_WIDTH / 2.0f) - (this->btn_save[0]->get_sprite_width() / 2.0f);
    float sprite_height = this->btn_save[0]->get_sprite_height();
    int pos_x = -15;

    this->btn_save[0]->draw_button(font, center_x, pos_x, 1.0f);
    this->btn_save[1]->draw_button(font, center_x, pos_x + sprite_height, 1.0f);
    this->btn_save[2]->draw_button(font, center_x, pos_x + (sprite_height * 2), 1.0f);
    this->btn_save[3]->draw_button(font, center_x, pos_x + (sprite_height * 3), 1.0f);
}

void SplashScreen::draw_create_player() {
    std::string temp_str = this->save_lst[this->save_slot_choice]->player_name;
    DrawFontText(this->font, temp_str, (SCREEN_WIDTH / 2.0f) - (GetFontTextWidth(temp_str, 2.0f) / 2.0f), 50, this->font_color, 2.0f);
}

static int last_character(char player_name[P_NAME_LEN]) {
    int position = 0;

    for (long unsigned int i = 0; i < std::strlen(player_name); i++) {
        if (!std::isalnum(player_name[i]) && player_name[i] != ' ')
            break;
        position += 1;
    }
    return position;
}
 
void SplashScreen::keyboard_input() {
    int key_got = GetKeyPressed();
    if (key_got > 0) {
        int char_position = last_character(this->save_lst[this->save_slot_choice]->player_name);

        if (key_got == KEY_BACKSPACE && char_position > 0) {
            this->save_lst[this->save_slot_choice]->player_name[char_position - 1] = '\0';
        }
        else if (char_position < P_NAME_LEN - 1)
            this->save_lst[this->save_slot_choice]->player_name[char_position] = key_got;
    }
}

