#include "splash_screen.hpp"
#include "helper.hpp"

static void change_to_sub_menu(int id, State &current_state) {
    if (id == 0)
        current_state = load_game;
    else if (id == 1)
        current_state = new_game;
}

static void create_or_launch(State &current_state, GameState *game_state, int &slot_choose, int id) {
    if (current_state == new_game) {
        current_state = create;
        slot_choose = id;
    } else {
        game_state->set_init_scene(true);
        game_state->set_actual_scene(game);
    }
}

SplashScreen::SplashScreen(GameState *game_state) {
    this->game_state = game_state;

    this->font = LoadTexture(GetAssetPath("test_font.png").c_str());
    this->mn_background = LoadTexture(GetAssetPath("main_menu.png").c_str());
    this->arrow = LoadTexture(GetAssetPath("arrow.png").c_str());
    this->btn_background = LoadTexture(GetAssetPath("button_save_select.png").c_str());
    this->btn_background_variant = LoadTexture(GetAssetPath("button_save_select_variant.png").c_str());

    font_color = Color{ 255, 255, 255, 255 };
    color_sens = false;
    current_state = welcome;
    selected_id = 0;
    save_slot_choice = 0;

    // Main menu buttons
    button_list.push_back(std::make_unique<Button>(&this->arrow, nullptr, "Load", [this]() {
        change_to_sub_menu(0, current_state);
    }));
    button_list.push_back(std::make_unique<Button>(&this->arrow, nullptr, "New game", [this]() {
        change_to_sub_menu(1, current_state);
    }));
    button_list.push_back(std::make_unique<Button>(&this->arrow, nullptr, "Option", [this]() {
        change_to_sub_menu(2, current_state);
    }));

    // Save slot buttons
    for (int i = 0; i <= 3; i++) {
        if (!is_slot_used(i)) {
            btn_save.push_back(std::make_unique<Button>(nullptr, &this->btn_background, "Empty", [this, i]() {
                create_or_launch(current_state, this->game_state, this->save_slot_choice, i);
            }));
            this->save_lst.push_back(std::make_unique<GameData>());
            this->save_lst.back()->player_name[P_NAME_LEN - 1] = '\0';
        } else {
            this->save_lst.push_back(std::make_unique<GameData>());
            load_data(i, *this->save_lst.back());
            std::string str(this->save_lst.back()->player_name);
            btn_save.push_back(std::make_unique<Button>(nullptr, &this->btn_background_variant, str, [this, i]() {
                create_or_launch(current_state, this->game_state, this->save_slot_choice, i);
            }));
        }
    }
}

SplashScreen::~SplashScreen() {
    UnloadTexture(font);
    UnloadTexture(mn_background);
    UnloadTexture(arrow);
    UnloadTexture(btn_background);
    UnloadTexture(btn_background_variant);
}

void SplashScreen::update() {
    float speed = 3.0f;
    if (font_color.b <= 50)
        color_sens = true;
    if (font_color.b >= 250)
        color_sens = false;

    if (!color_sens)
        font_color.b = static_cast<unsigned char>(font_color.b - speed);
    else
        font_color.b = static_cast<unsigned char>(font_color.b + speed);

    switch (current_state) {
        case welcome:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                current_state = main_menu;
            break;
        case main_menu:
            if (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                current_state = welcome;
            break;
        case new_game:
            if (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                current_state = main_menu;
            break;
        case create:
            if (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                current_state = new_game;
            SplashScreen::keyboard_input();
            if (IsKeyPressed(KEY_ENTER)) {
                save_game(this->save_slot_choice, *this->save_lst[this->save_slot_choice]);
                game_state->set_init_scene(true);
                game_state->set_actual_scene(game);
            }
            break;
        case load_game:
            if (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                current_state = main_menu;
            break;
        default:
            break;
    }
}

void SplashScreen::draw_welcome_screen() {
    DrawFontText(font, "Press start", SCREEN_WIDTH - 170, SCREEN_HEIGHT - 20, font_color, 1.0f);
}

void SplashScreen::draw_main_menu() {
    Rectangle src = { 0, 0, (float)mn_background.width, (float)mn_background.height };
    Rectangle dst = { SCREEN_WIDTH / 2.0f - 128.0f, SCREEN_HEIGHT / 2.0f - 128.0f, 256.0f, 256.0f };
    DrawTexturePro(mn_background, src, dst, Vector2{0, 0}, 0.0f, WHITE);

    this->button_list[0]->draw_button(font, (SCREEN_WIDTH - this->button_list[0]->get_size_text(1.0f)) / 2.0f, (SCREEN_HEIGHT / 2.0f) - (256.0f * 0.3f), 1.0f);
    this->button_list[1]->draw_button(font, (SCREEN_WIDTH - this->button_list[1]->get_size_text(1.0f)) / 2.0f, (SCREEN_HEIGHT / 2.0f) - (256.0f * 0.0f), 1.0f);
    this->button_list[2]->draw_button(font, (SCREEN_WIDTH - this->button_list[2]->get_size_text(1.0f)) / 2.0f, (SCREEN_HEIGHT / 2.0f) + (256.0f * 0.3f), 1.0f);
}

void SplashScreen::draw() {
    switch (current_state) {
        case welcome:
            draw_welcome_screen();
            break;
        case main_menu:
            draw_main_menu();
            break;
        case new_game:
            draw_new_game();
            break;
        case create:
            draw_create_player();
            break;
        case load_game:
            draw_load_game();
            break;
        default:
            break;
    }
}
