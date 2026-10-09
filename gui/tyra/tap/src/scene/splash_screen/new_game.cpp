#include "helper.hpp"
#include "splash_screen.hpp"
#include "helper.hpp""
#include <fcntl.h>
#include <cctype>
#include <cstring>


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
            pos_x = 20;
            pos_y += offset;
        }
    }
    std::string temp_str = this->save_lst[this->save_slot_choice]->player_name;
    draw_text(engine, temp_str, &this->font, (engine->renderer.core.getSettings().getWidth() / 2) - (static_cast<float>(get_text_len(temp_str, 2.0f / 2))), 50, 2.0f);
}

static int last_character_position(char str[P_NAME_LEN]) {
    int res = 0;
    for (int i = 0; i < strlen(str); i++) {
        if (!isalnum(str[i]))
            return res;
        res += 1;
    }
    return res;
}

void put_letter_in_player_name(int save_slot_choice, GameData &data, char c) {
    if (strlen(data.player_name) < P_NAME_LEN - 1) {
        data.player_name[last_character_position(data.player_name)] += c;
    }
}
