#pragma once
#include "game_scene.hpp"
#include "game_state.hpp"
#include "raylib.h"
#include "server.hpp"
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4242

enum GameWorldScene {
    connection = 0,
    connection_loading = 1,
    connection_error = 2,
    island_1 = 3,
};

class GameWorld : public GameScene {
public:
    int fd;
    std::string player_name;
    std::string error_log;
    NetCommand command;
    struct sockaddr_in server;
    GameWorldScene intern_scene;
    GameState *game_state;
    Texture2D font;
    void draw_loading_screen();
    void draw_connection_error();
    void draw_island_1();

    GameWorld(GameState *game_state);
    ~GameWorld();

    void update() override;
    void server_alive();
    void net_update();
    void draw() override;
};
