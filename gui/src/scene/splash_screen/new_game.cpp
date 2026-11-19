#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>

#include "inc/helper.hpp"
#include "inc/raylib.h"
#include "inc/splash_screen.hpp"

void SplashScreen::draw_new_game() {
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
    input_hint(this->font, this->mouse_keyboard_icons, "back");
}

void SplashScreen::draw_create_player() {
    std::string temp_str = this->save_lst[this->save_slot_choice]->player_name;
    std::string enter_name = "Enter your name:";

    DrawFontText(
        this->font, enter_name,
        (SCREEN_WIDTH / 2.0f) - (GetFontTextWidth(enter_name, 2.0f) / 2.0f), 20,
        WHITE, 2.0f);
    DrawFontText(
        this->font, temp_str,
        (SCREEN_WIDTH / 2.0f) - (GetFontTextWidth(temp_str, 2.0f) / 2.0f),
        static_cast<float>(SCREEN_HEIGHT) / 2, this->font_color, 2.0f);
    input_hint(this->font, this->mouse_keyboard_icons, "back");
}

static int last_character(char player_name[P_NAME_LEN]) {
    int position = 0;

    for (size_t i = 0; i < std::strlen(player_name); i++) {
        if (!std::isalnum(player_name[i]) && player_name[i] != '_')
            break;
        position += 1;
    }
    return position;
}

// GetCharPressed gives the typed character (case, Shift), not a key code.
// Only letters, digits and '_' are kept, like the server's username rules.
void SplashScreen::keyboard_input() {
    char *name = this->save_lst[this->save_slot_choice]->player_name;
    int len = last_character(name);

    if (IsKeyPressed(KEY_BACKSPACE) && len > 0)
        name[--len] = '\0';
    for (int c = GetCharPressed(); c > 0; c = GetCharPressed()) {
        if (len < P_NAME_LEN - 1 && c < 128 && (std::isalnum(c) || c == '_')) {
            name[len++] = static_cast<char>(c);
            name[len] = '\0';
        }
    }
}
