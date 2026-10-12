#include <fcntl.h>
#include <memory>
#include <tyra>
#include "splash_screen.hpp"
#include "debug/debug.hpp"
#include "game_state.hpp"
#include "helper.hpp"
#include "button.hpp"
#include "save_load.hpp"
#include "new_game.hpp"
#include "game.hpp"

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
        // current_state = create;
}

void create_or_launch(State &current_state, int &slot_choose, int &selected_id) {
    current_state = create;
    slot_choose = selected_id;
}

SplashScreen::SplashScreen(Tyra::Engine* engine, GameState *game_state) {
    this->engine = engine;
    // load_image(&background, engine, "oui.png");
    load_image(&font, engine, "test_font.png");
    load_image(&mn_background, engine, "main_menu.png");
    load_image(&arrow, engine, "arrow.png");
    load_image(&btn_background, engine, "button_save_select.png");
    load_image(&keyboard, engine, "keyboard.png");
    load_image(&keyboard_long, engine, "keyboard_long.png");
    load_image(&controller, engine, "controller_button.png");
    font_color.a = 128.0f;
    font_color.r = 128.0f;
    font_color.g = 128.0f;
    font_color.b = 128.0f;
    SplashScreen::load_background();
    selected_id = 0;
    save_slot_choice = 0;
    // main menu button
    button_list.push_back(std::make_unique<Button>(&this->arrow, nullptr, "Load", [this]() {change_to_sub_menu(selected_id, current_state);}, this->engine));
    button_list.push_back(std::make_unique<Button>(&this->arrow, nullptr, "New game", [this]() {change_to_sub_menu(selected_id, current_state);}, this->engine));
    button_list.push_back(std::make_unique<Button>(&this->arrow, nullptr, "Option", [this]() {change_to_sub_menu(selected_id, current_state);}, this->engine));
    // new game button
    for (int i = 0; i <= 3; i++) {
        if (is_slot_used(i) == false) {
            btn_save.push_back(std::make_unique<Button>(nullptr, &this->btn_background, "Empty", [this]() {create_or_launch(current_state, this->save_slot_choice, this->selected_id);}, this->engine));
            this->save_lst.push_back(std::make_unique<GameData>());
            this->save_lst.back()->player_name[P_NAME_LEN - 1] = '\0';
        }
        else {
            this->save_lst.push_back(std::make_unique<GameData>());
            load_data(i, *this->save_lst.back());
            std::string str(this->save_lst.back()->player_name);
            TYRA_LOG(str);
            btn_save.push_back(std::make_unique<Button>(nullptr, &this->btn_background, str, [this]() {create_or_launch(current_state, this->save_slot_choice, this->selected_id);}, this->engine));
        }
    }
    // player creation keyboard
    for (int i = 65; i <= 90; i++) {
        std::string str(1, static_cast<char>(i));
        btn_keyboard.push_back(std::make_unique<Button>(nullptr, &this->keyboard, str, [this, i]() {put_letter_in_player_name(*this->save_lst[this->save_slot_choice], static_cast<char>(i));}, this->engine));
    }
    for (int i = 48; i <= 57; i++) {
        std::string str(1, static_cast<char>(i));
        btn_keyboard.push_back(std::make_unique<Button>(nullptr, &this->keyboard, str, [this, i]() {put_letter_in_player_name(*this->save_lst[this->save_slot_choice], static_cast<char>(i));}, this->engine));
    }
    TYRA_LOG("SPLASH_SCREEN: all assets loaded in memory");
}

SplashScreen::~SplashScreen() {
    this->engine->renderer.getTextureRepository().freeBySprite(background);
    this->engine->renderer.getTextureRepository().freeBySprite(mn_background);
    this->engine->renderer.getTextureRepository().freeBySprite(font);
    this->engine->renderer.getTextureRepository().freeBySprite(arrow);
    this->engine->renderer.getTextureRepository().freeBySprite(btn_background);
    this->engine->renderer.getTextureRepository().freeBySprite(keyboard);
    this->engine->renderer.getTextureRepository().freeBySprite(keyboard_long);
}

void clear_button_selection(std::vector<std::unique_ptr<Button>> &button_list) {
    for (int i = 0; i < button_list.size(); i++) {
        button_list[i]->set_is_selected(false);
    }
}

void SplashScreen::update() {
    const float joy_center_value = 128.0f;
    float speed = 3.0f;
    auto &pad = engine->pad;

    if (font_color.b <= 0)
        color_sens = true;
    if (font_color.b >= joy_center_value)
        color_sens = false;
    if (color_sens == false)
        font_color.b -= speed;
    else
        font_color.b += speed;
    clear_button_selection(button_list);
    clear_button_selection(btn_save);
    clear_button_selection(btn_keyboard);
    switch (current_state) {
        case welcome:
            if (pad.getClicked().Start)
                current_state = main_menu;
            break;
        case main_menu: {
            loop_button_lst(this->engine, this->button_list, this->selected_id, this->is_left_joy_centered, 1, 3);
            if (pad.getClicked().Triangle) {
                current_state = welcome;
                this->selected_id = 0;
            }
            if (pad.getClicked().Cross)
                this->selected_id = 0;
            break;
        }
        case new_game:
            loop_button_lst(this->engine, this->btn_save, this->selected_id, this->is_left_joy_centered, 1, 4);
            if (pad.getClicked().Triangle) {
                current_state = main_menu;
                this->selected_id = 0;
            }
            if (pad.getClicked().Cross)
                this->selected_id = 0;
            break;
        case create:
            loop_button_lst(this->engine, this->btn_keyboard, this->selected_id, this->is_left_joy_centered, 12, 3);
            keyboard_input(pad, this->save_slot_choice, *this->save_lst[this->save_slot_choice]);
            if (pad.getClicked().Triangle) {
                current_state = new_game;
                this->selected_id = 0;
            }
            if (pad.getClicked().Start) {
                int res = save_game(this->save_slot_choice, *this->save_lst[this->save_slot_choice]);
                TYRA_LOG("SAVE: Status", res);
                game_state.set_game_scene(new GameWorld());
            }
            break;
        case load_game:
            if (pad.getClicked().Triangle)
                current_state = main_menu;
            break;
        default:
            break;
    }
}

void SplashScreen::draw_welcome_screen() {
    const auto& screenSettings = this->engine->renderer.core.getSettings();
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
        case create:
            SplashScreen::draw_create_player();
            break;
        case load_game:
            SplashScreen::draw_load_game();
        default:
            break;
    }
}
