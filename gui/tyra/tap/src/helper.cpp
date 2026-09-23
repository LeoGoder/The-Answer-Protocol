#include <tyra>


void load_image(Tyra::Sprite *sprite, Tyra::Engine* engine, std::string img_path) {
    auto &renderer = engine->renderer;
    auto &textureRepository = renderer.getTextureRepository();
    auto filepath = Tyra::FileUtils::fromCwd(img_path);
    auto *texture = textureRepository.add(filepath);
    texture->addLink(sprite->id);
}
