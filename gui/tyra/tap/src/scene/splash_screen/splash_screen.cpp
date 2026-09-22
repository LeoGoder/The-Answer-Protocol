#include <tyra>
#include "splash_screen.hpp"

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
    auto &renderer = this->engine->renderer;
    auto &textureRepository = renderer.getTextureRepository();
    auto filepath = Tyra::FileUtils::fromCwd("oui.png");
    auto *texture = textureRepository.add(filepath);
    texture->addLink(background.id);
    SplashScreen::load_background();
    TYRA_LOG("background loaded");
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
