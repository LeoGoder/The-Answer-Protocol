#include <tyra>
#include <unistd.h>
#include <libmc.h>
#include <loadfile.h>
#include "tap.hpp"
#include "debug/debug.hpp"
#include "game_state.hpp"
#include "splash_screen.hpp"
#include "time/timer.hpp"

// IRX modules
extern "C" {
extern unsigned char mcman_irx[];
extern unsigned int size_mcman_irx;
extern unsigned char mcserv_irx[];
extern unsigned int size_mcserv_irx;
}

namespace Tyra {

Tap::Tap(Engine* t_engine) : engine(t_engine), game_state() {}

Tap::~Tap() {
    engine->renderer.getTextureRepository().freeBySprite(sprite);
}

void Tap::init() {
    int ret = 0;
    int mod_res = 0;

    // --- ANCIENNES TENTATIVES (gardees en commentaire au cas ou) ---
    // Tentative 1: Chargement direct des modules ROM standard (echoue avec -203 sur SCPH-90004)
    // ret = SifLoadModule("rom0:MCMAN", 0, nullptr);
    // TYRA_LOG("rom0:MCMAN: ret=", ret);
    // ret = SifLoadModule("rom0:MCSERV", 0, nullptr);
    // TYRA_LOG("rom0:MCSERV: ret=", ret);

    // Tentative 2: Chargement direct des modules ROM etendus (echoue avec -203 sur SCPH-90004 Deckard)
    // ret = SifLoadModule("rom0:XMCMAN", 0, nullptr);
    // TYRA_LOG("rom0:XMCMAN: ret=", ret);
    // ret = SifLoadModule("rom0:XMCSERV", 0, nullptr);
    // TYRA_LOG("rom0:XMCSERV: ret=", ret);

    // --- METHODE PAR BUFFER AVEC GESTION DU BUS SIO2 ---
    // 1. Fermer temporairement le port de la manette pour liberer le bus SIO2
    padPortClose(0, 0);

    ret = SifExecModuleBuffer(mcman_irx, size_mcman_irx, 0, nullptr, &mod_res);
    TYRA_LOG("MCMAN buffer: ret=", ret, ", mod_res=", mod_res);

    ret = SifExecModuleBuffer(mcserv_irx, size_mcserv_irx, 0, nullptr, &mod_res);
    TYRA_LOG("MCSERV buffer: ret=", ret, ", mod_res=", mod_res);

    int res = mcInit(MC_TYPE_XMC);
    TYRA_LOG("mcInit: res=", res);

    engine->pad.init();

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
    // TYRA_LOG("DT: ", game_state.dt);
}

void Tap::loadSprite() {
    const auto& screenSettings = engine->renderer.core.getSettings();

    sprite.mode = SpriteMode::MODE_STRETCH;
    sprite.size = Vec2(256.0F, 256.0F);
    sprite.position =
        Vec2(screenSettings.getWidth() / 2.0F - sprite.size.x / 2.0F,
             screenSettings.getHeight() / 2.0F - sprite.size.y / 2.0F);
    TYRA_LOG("Sprite created!");
}

void Tap::loadTexture() {
    TYRA_LOG("Texture loaded!");
}

}  // namespace Tyra
