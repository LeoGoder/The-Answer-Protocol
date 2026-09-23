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
    load_image(&background, engine, "oui.png");
    SplashScreen::load_background();
    TYRA_LOG("all assets loaded in memory");
}

SplashScreen::~SplashScreen() {
    this->engine->renderer.getTextureRepository().freeBySprite(background);
}

void SplashScreen::update() {

}

void SplashScreen::draw() {
    auto& renderer = engine->renderer;
    renderer.renderer2D.render(background);
}
