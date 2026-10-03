#include <tyra>
#include "splash_screen.hpp"
#include "debug/debug.hpp"
#include "helper.hpp"
#include "button.hpp"
#include "save_load.hpp"

void SplashScreen::load_background() {
    const auto& screenSettings = this->engine->renderer.core.getSettings();

    background.mode = Tyra::SpriteMode::MODE_STRETCH;
    background.size = Tyra::Vec2(256.0F, 256.0F);
    background.position =
        Tyra::Vec2(screenSettings.getWidth() / 2.0F - background.size.x / 2.0F,
             screenSettings.getHeight() / 2.0F - background.size.y / 2.0F);
    TYRA_LOG("background created!");
}

void change_to_sub_menu(int selected_id, State &current_state) {
    if (selected_id == 0)
        current_state = load_game;
    else if (selected_id == 1)
        current_state = new_game;
    // else if (selected_id == 2)
    //     current_state = new_game;
}

void temp_func() {
    TYRA_LOG("salam 2 le retour");
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
    button_list.push_back(std::make_unique<Button>("Load", "None", [this]() {change_to_sub_menu(selected_id, current_state);}, this->engine));
    button_list.push_back(std::make_unique<Button>("New game", "None", [this]() {change_to_sub_menu(selected_id, current_state);}, this->engine));
    button_list.push_back(std::make_unique<Button>("Option", "None", [this]() {change_to_sub_menu(selected_id, current_state);}, this->engine));
    for (int i = 1; i <= 4; i++) {
        if (is_slot_used(i) == false)
            btn_save.push_back(std::make_unique<Button>("Empty", "button_save_select.png", [this]() {temp_func();}, this->engine));
    }
    TYRA_LOG("all assets loaded in memory");
}

SplashScreen::~SplashScreen() {
    this->engine->renderer.getTextureRepository().freeBySprite(background);
    this->engine->renderer.getTextureRepository().freeBySprite(mn_background);
    this->engine->renderer.getTextureRepository().freeBySprite(font);
    for (int i = 0; i < 3; i++) {
        if (button_list[i]->get_img_path() != "None")
            this->engine->renderer.getTextureRepository().freeBySprite(button_list[i]->get_sprite());
    }
}

void SplashScreen::update() {
    const float joy_center_value = 128.0f;
    float speed = 3.0f;
    if (font_color.b <= 0)
        color_sens = true;
    if (font_color.b >= joy_center_value)
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
            loop_button_lst(this->engine, this->button_list, this->selected_id, this->is_left_joy_centered);
            break;
        }
        case new_game:
            if (pad.getClicked().Circle) {
                current_state = main_menu;
                this->btn_save[this->selected_id]->set_is_selected(false);
                this->selected_id = 0;
            }
            loop_button_lst(this->engine, this->btn_save, this->selected_id, this->is_left_joy_centered);
            break;
        case load_game:
            if (pad.getClicked().Circle)
                current_state = main_menu;
            break;
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
    draw_sprite(this->engine, this->mn_background, screenSettings.getWidth() / 2.0F - mn_background.size.x / 2.0F, screenSettings.getHeight() / 2.0F - mn_background.size.y / 2.0F, 256.0f, 256.0f);
    this->engine->renderer.renderer2D.render(mn_background);
    // draw here all options possible
    this->button_list[0]->draw_button(&font, (screenSettings.getWidth() - this->button_list[0]->get_size_text(1.0f)) / 2.0f, (screenSettings.getHeight() / 2) - (mn_background.size.y * 0.3f), 1.0f);
    this->button_list[1]->draw_button(&font, (screenSettings.getWidth() - this->button_list[1]->get_size_text(1.0f)) / 2.0f, (screenSettings.getHeight() / 2) - (mn_background.size.y * 0.0f), 1.0f);
    this->button_list[2]->draw_button(&font, (screenSettings.getWidth() - this->button_list[2]->get_size_text(1.0f)) / 2.0f, (screenSettings.getHeight() / 2) + (mn_background.size.y * 0.3f), 1.0f);
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
        case new_game:
            SplashScreen::draw_new_game();
            break;
        case load_game:
            SplashScreen::draw_load_game();
        default:
            break;
    }
}
