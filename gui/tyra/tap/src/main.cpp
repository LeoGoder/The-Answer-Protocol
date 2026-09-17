#include "engine.hpp"
#include "tap.hpp"

int main() {
  Tyra::Engine engine;
  Tyra::Tap game(&engine);
  engine.run(&game);
  return 0;
}
