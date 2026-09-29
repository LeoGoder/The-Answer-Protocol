#include <tyra>
#include <unistd.h>
#include "tap.hpp"
#include "debug/debug.hpp"
#include "game_state.hpp"
#include "splash_screen.hpp"
#include "time/timer.hpp"

namespace Tyra {

Tap::Tap(Engine* t_engine) : engine(t_engine), game_state() {}

Tap::~Tap() {
    engine->renderer.getTextureRepository().freeBySprite(sprite);
}

void Tap::init() {
    engine->renderer.setClearScreenColor(Color(32.0F, 32.0F, 32.0F));
    game_state.last_time = game_state.timer.getTimeDelta();
    engine->renderer.core.renderer2D.setTextureMappingType(game_state.texture_filter);
}

void Tap::loop() {
    auto& renderer = engine->renderer;
    GameScene *game_scene = game_state.get_game_scene();
    Scene scene = game_state.get_actual_scene();

    renderer.beginFrame();
    if (game_state.get_init_scene() == true) {
        if (game_scene != nullptr) {
            delete game_scene;
        }
        switch(scene) {
            case splash_screen:
                game_state.set_game_scene(new SplashScreen(engine));
                break;
            case game:
                break;
            case end_screen:
                break;
            default:
                break;
        }
        game_state.set_init_scene(false);
        game_scene = game_state.get_game_scene();
    }
    if (game_scene != nullptr) {
        game_scene->update();
        game_scene->draw();
    }
    renderer.endFrame();
    u32 current_time = game_state.timer.getTimeDelta();
    u16 temp_dt = static_cast<u16>(current_time - game_state.last_time);
    game_state.last_time = current_time;
    game_state.dt = static_cast<float>(temp_dt) / 15625.0f;
    TYRA_LOG("DT: ", game_state.dt);
}
}  // namespace Tyra
