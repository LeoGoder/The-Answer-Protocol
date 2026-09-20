#pragma once

enum Scene {
    splash_screen = 0,
    main_menu = 1,
    settings = 2, // welcome back settings menu
};

class GameState {
    private:
        Scene actual_scene;
        Scene next_scene;

    public:
        GameState();
        ~GameState();
        void set_actual_scene(Scene new_scene) {
            actual_scene = new_scene;
        }

        void set_next_scene(Scene new_scene) {
            next_scene = new_scene;
        }

        Scene get_actual_scene() const {
            return actual_scene;
        }

        Scene get_next_scene() const {
            return next_scene;
        }
};
