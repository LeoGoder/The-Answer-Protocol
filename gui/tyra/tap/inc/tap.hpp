#pragma once

#include <tyra>

namespace Tyra {

class Tap : public Game {
 public:
  Tap(Engine* engine);
  ~Tap();

  void init();
  void loop();

 private:
  void loadTexture();
  void loadSprite();

  Engine* engine;
  Sprite sprite;
};

}  // namespace Tyra
