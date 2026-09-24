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
    TYRA_LOG("all assets loaded in memory");
}

SplashScreen::~SplashScreen() {
    this->engine->renderer.getTextureRepository().freeBySprite(background);
    this->engine->renderer.getTextureRepository().freeBySprite(font);
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
        case main_menu:
            break;
        default:
            break;
    }
}

void SplashScreen::draw_welcome_screen() {
    const auto& screenSettings = this->engine->renderer.core.getSettings();
    auto& renderer = engine->renderer;
    // renderer.renderer2D.render(background);
    draw_text_color(this->engine, "Press start", &font, screenSettings.getWidth() - 170, screenSettings.getHeight() - 20, font_color, 1.0f);
}

void SplashScreen::draw_main_menu() {
    const auto& screenSettings = this->engine->renderer.core.getSettings();
    mn_background.mode = Tyra::SpriteMode::MODE_STRETCH;
    mn_background.size = Tyra::Vec2(256.0f, 256.0f);
    mn_background.position =
        Tyra::Vec2(screenSettings.getWidth() / 2.0F - mn_background.size.x / 2.0F,
             screenSettings.getHeight() / 2.0F - mn_background.size.y / 2.0F);
    this->engine->renderer.renderer2D.render(mn_background);
}

void SplashScreen::draw() {
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
