#pragma once
#include "game_scene.hpp"
#include "renderer/3d/pipeline/shared/pipeline_texture_mapping_type.hpp"

enum Scene {
    splash_screen = 0,
    main_menu = 1,
    settings = 2, // welcome back settings menu
};

class GameState {
    private:
        Scene actual_scene;
        Scene next_scene;
        bool init_scene;
        GameScene *game_scene = nullptr;
    public:
        Tyra::PipelineTextureMappingType texture_filter = Tyra::TyraNearest;
        GameState();
        ~GameState();
        void set_actual_scene(Scene new_scene) {
            actual_scene = new_scene;
        }

        void set_next_scene(Scene new_scene) {
            next_scene = new_scene;
        }

        void set_game_scene(GameScene *new_game_scene) {
            game_scene = new_game_scene;
        }

        void set_init_scene(bool state) {
            init_scene = state;
        }

        Scene get_actual_scene() const {
            return actual_scene;
        }

        Scene get_next_scene() const {
            return next_scene;
        }

        GameScene *get_game_scene() {
            return game_scene;
        }

        bool get_init_scene() const {
            return init_scene;
        }
};
