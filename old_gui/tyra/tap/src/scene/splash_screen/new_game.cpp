#include "helper.hpp"
#include "pad/pad.hpp"
#include "save_load.hpp"
#include "splash_screen.hpp"
#include "helper.hpp"
#include <cstddef>
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
    float command_offset = 5;
    const std::string space = "Space";
    const std::string delete_char = "Delete";
    const std::string start = "Start";
    unsigned int btn_key_size = this->btn_keyboard.size();

    for (unsigned int i = 0; i < btn_key_size; i++) {
        this->btn_keyboard[i]->draw_button(&this->font, pos_x, pos_y, 2.0f);
        pos_x += offset;
        if (pos_x >= (default_pos_x + (offset * 12))) {
            pos_x = 20;
            pos_y += offset;
        }
    }
    float last_button_size = btn_background.size.x / 2 + 10;
    pos_y += 20;
    pos_x = 20;

    draw_sprite_sheet(this->engine, this->controller, pos_x, pos_y, 32.0f, 32.0f, 1, 4, 1.0f);
    draw_text(this->engine, space, &this->font, pos_x + 36.0f, pos_y + 12.0f, 1.0f);
    pos_x = pos_x + 36.0f + get_text_len(space, 1.0f) + command_offset;

    draw_sprite_sheet(this->engine, this->controller, pos_x, pos_y, 32.0f, 32.0f, 3, 4, 1.0f);
    draw_text(this->engine, delete_char, &this->font, pos_x + 36.0f, pos_y + 12.0f, 1.0f);
    pos_x = pos_x + 36.0f + get_text_len(delete_char, 1.0f) + command_offset;

    draw_sprite_sheet(this->engine, this->controller, pos_x, pos_y, 32.0f, 32.0f, 4, 4, 1.0f);
    draw_text(this->engine, start, &this->font, pos_x + 36.0f, pos_y + 12.0f, 1.0f);
    pos_x = pos_x + 36.0f + get_text_len(start, 1.0f) + command_offset;

    draw_sprite_sheet(this->engine, this->controller, pos_x, pos_y, 32.0f, 32.0f, 5, 4, 1.0f);
    draw_text(this->engine, "Clear", &this->font, pos_x + 36.0f, pos_y + 12.0f, 1.0f);

    std::string temp_str = this->save_lst[this->save_slot_choice]->player_name;
    draw_text_color(engine, temp_str, &this->font, (engine->renderer.core.getSettings().getWidth() / 2) - (static_cast<float>(get_text_len(temp_str, 2.0f / 2))), 50, this->font_color, 2.0f);
}

static int last_character_position(char str[P_NAME_LEN]) {
    int res = 0;
    for (size_t i = 0; i < strlen(str); i++) {
        if (!isalnum(str[i]) && str[i] != ' ')
            return res;
        res += 1;
    }
    return res;
}

void delete_last_character(GameData &data) {
    int last_char = last_character_position(data.player_name) - 1;
    if (last_char >= 0)
        data.player_name[last_char] = '\0';
}

void clear_all_character(GameData &data) {
    for (int i = 0; i < P_NAME_LEN; i++) {
        data.player_name[i] = '\0';
    }
}

void put_letter_in_player_name(GameData &data, char c) {
    if (strlen(data.player_name) < P_NAME_LEN - 1) {
        if (c == ' ')
            data.player_name[last_character_position(data.player_name)] = c;
        else
            data.player_name[last_character_position(data.player_name)] = c;
    }
}

void keyboard_input(Tyra::Pad &pad, int save_slot_choice, GameData &data) {

    if (pad.getClicked().Circle)
        delete_last_character(data);
    if (pad.getClicked().Square)
        put_letter_in_player_name(data, ' ');
    if (pad.getClicked().Select)
        clear_all_character(data);
}
