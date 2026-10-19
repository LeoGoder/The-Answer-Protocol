#include "tap.hpp"
#include "helper.hpp"
#include "raylib.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, "The Answer Protocol");
    SetTargetFPS(60);
    SetExitKey(0);
    {
        Tap game;
        Shader post_process = LoadShader(0, GetAssetPath("old_screen_shaders.fs").c_str());
        RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
        int resolutionLoc = GetShaderLocation(post_process, "resolution");
        int timeLoc       = GetShaderLocation(post_process, "time");
        Vector2 screenRes = { (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };

        SetShaderValue(post_process, resolutionLoc, &screenRes, SHADER_UNIFORM_VEC2);
        game.init();
        while (!WindowShouldClose()) {
            float time = (float)GetTime();

            SetShaderValue(post_process, timeLoc, &time, SHADER_UNIFORM_FLOAT);
            BeginTextureMode(target);
                ClearBackground(Color{ 32, 32, 32, 255 });
                game.loop();
            EndTextureMode();

            BeginDrawing();
                ClearBackground(Color{ 32, 32, 32, 255 });
                BeginShaderMode(post_process);
                    DrawTexturePro(target.texture,
                                    (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height},
                                    (Rectangle){0, 0, (float)GetScreenWidth(), (float)-GetScreenHeight()},
                                    Vector2{0, 0},
                                    0.0f,
                                    WHITE);
                EndShaderMode();
            EndDrawing();
        }
        UnloadRenderTexture(target);
        UnloadShader(post_process);
    }
    CloseWindow();
    return 0;
}
