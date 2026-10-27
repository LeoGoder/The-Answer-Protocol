#pragma once
#include "game_scene.hpp"
#include "game_state.hpp"
#include "map_parse.hpp"
#include "raylib.h"
#include "server.hpp"
#include "player.hpp"
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4242
#define ALIVE_MAX_FAIL 6

enum GameWorldScene {
    connection = 0,
    connection_loading = 1,
    connection_error = 2,
    in_game = 3,
};

class GameWorld : public GameScene {
    public:
        int fd;
        int alive_nb_fail;
        float alive_check_timer;
        float alive_timer;
        std::string player_name;
        std::string error_log;
        NetCommand command;
        EvtCommand event;
        struct sockaddr_in server;
        GameWorldScene intern_scene;
        GameState *game_state;
        Texture2D font;
        Texture2D player_sprite;
        Texture2D ground;
        Texture2D exterior;
        Texture2D calf;
        Texture2D fox;
        Texture2D rock_flowers;
        Texture2D sheep;
        Texture2D trees;
        Texture2D hare;
        Room room;
        Player player;
        Maps map;
        MapData map_data;

        GameWorld(GameState *game_state);
        ~GameWorld();

        void update() override;
        void server_alive(std::vector<std::string> msg_split);
        void net_update();
        void room_presence_enter(std::string new_player);
        void room_presence_leave(std::string delete_player);
        void draw() override;
        void draw_loading_screen();
        void draw_connection_error();
        void draw_game();
        void draw_hud();
        void draw_island(MapData &island);
};
