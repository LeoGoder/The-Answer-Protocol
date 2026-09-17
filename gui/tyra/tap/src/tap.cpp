#include <tyra>
#include "tap.hpp"

namespace Tyra {

Tap::Tap(Engine* t_engine) : engine(t_engine) {}

Tap::~Tap() {
    engine->renderer.getTextureRepository().freeBySprite(sprite);
}

void Tap::init() {
    engine->renderer.setClearScreenColor(Color(32.0F, 32.0F, 32.0F));

    loadSprite();
    loadTexture();
}

void Tap::loop() {
    auto& renderer = engine->renderer;

    renderer.beginFrame();

    renderer.renderer2D.render(sprite);

    renderer.endFrame();
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
    /**
     * Renderer has high layer functions,
     * which allows to render:
     * - Sprite (2D)
     * - Mesh (3D)
     *
     * It uses ONLY low layer functions which are in renderer.core
     */
    auto& renderer = engine->renderer;

    /**
     * TextureRepository is a repository of textures.
     * It is a singleton class, with all game textures.
     * We are linking these textures with sprite's (2D) and mesh (3D) materials.
     */
    auto& textureRepository = renderer.getTextureRepository();

    /**
     * Texture is stored in "res" directory.
     * Content of "res" directory is automatically copied into
     * "bin" directory, which contains our final game.
     *
     * File utils automatically add's device prefix to the path,
     * based on current working directory.
     *
     * In PS2 world:
     * - USB has a "mass:" prefix
     * - Our PC in PS2Link has a "host:" prefix
     * - Our PC in PCSX2 has a "host:" prefix
     */
    auto filepath = FileUtils::fromCwd("oui.png");

    /**
     * Tyra supports following PNG formats:
     * 32bpp (RGBA)
     * 24bpp (RGB)
     * 8bpp, palletized (RGBA)
     * 4bpp, palletized (RGBA)
     *
     * 8bpp and 4bpp are the fastest.
     * All of these formats can be easily exported via GIMP.
     */
    auto* texture = textureRepository.add(filepath);

    /** Let's assign this texture to sprite. */
    texture->addLink(sprite.id);

    TYRA_LOG("Texture loaded!");
}

}  // namespace Tyra
