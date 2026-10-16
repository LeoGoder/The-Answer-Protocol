#pragma once
#include "game_scene.hpp"
#include "game_state.hpp"
#include "raylib.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "192.168.1.42"
#define SERVER_PORT 4242

enum GameWorldScene {
    connection = 0,
    connection_error = 1,
    island_1 = 2,
};

class GameWorld : public GameScene {
public:
    int fd;
    struct sockaddr_in server;
    GameWorldScene intern_scene;
    GameState *game_state;
    Texture2D font;

    GameWorld(GameState *game_state);
    ~GameWorld();

    void update() override;
    void draw() override;
};
