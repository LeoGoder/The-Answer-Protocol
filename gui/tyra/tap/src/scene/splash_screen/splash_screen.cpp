#include <tyra>
#include "splash_screen.hpp"
#include "debug/debug.hpp"
#include "helper.hpp"

void SplashScreen::load_background() {
    const auto& screenSettings = this->engine->renderer.core.getSettings();

    background.mode = Tyra::SpriteMode::MODE_STRETCH;
    background.size = Tyra::Vec2(256.0F, 256.0F);
    background.position =
        Tyra::Vec2(screenSettings.getWidth() / 2.0F - background.size.x / 2.0F,
             screenSettings.getHeight() / 2.0F - background.size.y / 2.0F);
    TYRA_LOG("background created!");
}

// temp function
void temp_function() {
    TYRA_LOG("salam");
}

SplashScreen::SplashScreen(Tyra::Engine* engine) {
    this->engine = engine;
    // load_image(&background, engine, "oui.png");
    load_image(&font, engine, "test_font.png");
    load_image(&mn_background, engine, "main_menu.png");
    font_color.a = 128.0f;
    font_color.r = 128.0f;
    font_color.g = 128.0f;
    font_color.b = 128.0f;
    SplashScreen::load_background();
    selected_id = 0;
    button_list.push_back(Button("Load", "None", temp_function, this->engine));
    button_list.push_back(Button("New game", "None", temp_function, this->engine));
    button_list.push_back(Button("option", "None", temp_function, this->engine));
    TYRA_LOG("all assets loaded in memory");
}

SplashScreen::~SplashScreen() {
    this->engine->renderer.getTextureRepository().freeBySprite(background);
    this->engine->renderer.getTextureRepository().freeBySprite(mn_background);
    this->engine->renderer.getTextureRepository().freeBySprite(font);
    // selected_id = 0;
    // for (int i = 0; i < 3; i++) {
    //     button_list.push_back(Button("Load", "None", temp_function));
    // }
}

void SplashScreen::update() {
    float speed = 3.0f;
    if (font_color.b <= 0)
        color_sens = true;
    if (font_color.b >= 128.0f)
        color_sens = false;
    if (color_sens == false)
        font_color.b -= speed;
    else
        font_color.b += speed;
    auto &pad = engine->pad;
    switch (current_state) {
        case welcome:
            if (pad.getClicked().Start)
                current_state = main_menu;
            break;
        case main_menu: {
            this->button_list[this->selected_id].set_is_selected(false);
            int left_joy_v = pad.getLeftJoyPad().v;
            bool analog_down = false;
            bool analog_up = false;

            if (left_joy_v > 190 && this->is_left_joy_centered) {
                analog_down = true;
                this->is_left_joy_centered = false;
            }
            else if (left_joy_v < 60 && this->is_left_joy_centered) {
                analog_up = true;
                this->is_left_joy_centered = false;
            }
            else if (left_joy_v >= 60 && left_joy_v <=  190)
                this->is_left_joy_centered = true;

            if (pad.getClicked().Cross)
                this->button_list[this->selected_id].on_click();
            if (pad.getClicked().DpadDown || analog_down) {
                this->selected_id += 1;
                if (this->selected_id > this->button_list.size() - 1)
                    this->selected_id = 0;
            }
            if (pad.getClicked().DpadUp || analog_up) {
                this->selected_id -= 1;
                if (this->selected_id < 0)
                    this->selected_id = this->button_list.size() - 1;
            }
            this->button_list[this->selected_id].set_is_selected(true);
            break;
        }
        default:
            break;
    }
}

void SplashScreen::draw_welcome_screen() {
    const auto& screenSettings = this->engine->renderer.core.getSettings();
    auto& renderer = engine->renderer;
    draw_text_color(this->engine, "Press start", &font, screenSettings.getWidth() - 170, screenSettings.getHeight() - 20, font_color, 1.0f);
}

void SplashScreen::draw_main_menu() {
    const auto& screenSettings = this->engine->renderer.core.getSettings();
    const std::string load = "Load";
    const std::string new_game = "New game";
    const std::string option = "Options";
    mn_background.mode = Tyra::SpriteMode::MODE_STRETCH;
    mn_background.size = Tyra::Vec2(256.0f, 256.0f);
    mn_background.position =
        Tyra::Vec2(screenSettings.getWidth() / 2.0F - mn_background.size.x / 2.0F,
             screenSettings.getHeight() / 2.0F - mn_background.size.y / 2.0F);
    this->engine->renderer.renderer2D.render(mn_background);
    // draw here all options possible 
    this->button_list[0].draw_button(&font, (screenSettings.getWidth() - this->button_list[0].get_size_text(1.0f)) / 2.0f, (screenSettings.getHeight() / 2) - (mn_background.size.y * 0.3f), 1.0f);
    this->button_list[1].draw_button(&font, (screenSettings.getWidth() - this->button_list[1].get_size_text(1.0f)) / 2.0f, (screenSettings.getHeight() / 2) - (mn_background.size.y * 0.0f), 1.0f);
    this->button_list[2].draw_button(&font, (screenSettings.getWidth() - this->button_list[2].get_size_text(1.0f)) / 2.0f, (screenSettings.getHeight() / 2) + (mn_background.size.y * 0.3f), 1.0f);
}

void SplashScreen::draw() {
    // renderer.renderer2D.render(background);
    switch (current_state) {
        case welcome:
            SplashScreen::draw_welcome_screen();
            break;
        case main_menu:
            SplashScreen::draw_main_menu();
            break;
        default:
            break;
    }
}
