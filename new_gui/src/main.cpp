#include "tap.hpp"
#include "helper.hpp"
#include "raylib.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "The Answer Protocol");
    SetTargetFPS(60);

    Tap game;
    game.init();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color{ 32, 32, 32, 255 });
        game.loop();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
