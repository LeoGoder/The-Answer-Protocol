#pragma once
#include "game_scene.hpp"
#include "game_state.hpp"
#include "raylib.h"
#include "server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4242

enum GameWorldScene {
    connection = 0,
    connection_error = 1,
    island_1 = 2,
};

class GameWorld : public GameScene {
public:
    int fd;
    std::string player_name;
    NetCommand command;
    struct sockaddr_in server;
    GameWorldScene intern_scene;
    GameState *game_state;
    Texture2D font;

    GameWorld(GameState *game_state);
    ~GameWorld();

    void update() override;
    void draw() override;
};
