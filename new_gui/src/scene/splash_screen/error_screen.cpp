#include "helper.hpp"
#include "raylib.h"
#include "splash_screen.hpp"

void SplashScreen::draw_error_screen() {
    std::string text = "Error name invalid";
    float scale = 1.5f;
    float text_len =  GetFontTextWidth(text, scale);
    Color red_transparent = ColorAlpha(RED, 0.3);

    DrawFontText(this->font, text, (float(SCREEN_WIDTH) / 2) - (text_len / 2), float(SCREEN_HEIGHT) / 2, RED, scale);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, red_transparent);
    input_hint(this->font, this->mouse_keyboard_icons, "back");
}

