#pragma once
#include "game_scene.hpp"
#include "game_state.hpp"
#include "renderer/core/2d/sprite/sprite.hpp"
#include <tyra>

#define SERVER_IP "192.168.1.42"
#define SERVER_PORT 4242

extern "C" {
    #include <kernel.h>
    #include <ps2ip.h>
    extern unsigned char ps2dev9_irx[];
    extern unsigned int size_ps2dev9_irx;
    extern unsigned char netman_irx[];
    extern unsigned int size_netman_irx;
    extern unsigned char smap_irx[];
    extern unsigned int size_smap_irx;
}

enum GameWorldScene {
    connection = 0,
    connection_error = 1,
    island_1 = 2,
};

struct NetCommands {

};

struct NetEvents {

};

class GameWorld : public GameScene {
    private:
        int temp;
    public:
        // int frame = 0;
        // int frame_to_wait = 4;
        int fd;
        struct sockaddr_in server;
        GameWorldScene intern_scene = connection;
        Tyra::Engine *engine;
        GameState *game_state;
        GameWorld(Tyra::Engine *engine, GameState *game_state);
        ~GameWorld();
        Tyra::Sprite font;
        void update() override;
        void draw() override;
};

