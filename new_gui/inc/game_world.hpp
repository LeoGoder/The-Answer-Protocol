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
#define ALIVE_MAX_TIME 35
#define ALIVE_CHECK_TIMER 10
#define HUD_FONT_SIZE 10
#define HUD_PADDING 6
#define OBJECT_SIZE 32
#define OBJECT_SPACING 96
#define NPC_ROW_OFFSET 80
#define ITEM_ROW_OFFSET 160
#define INVENTORY_SLOTS 7
#define INVENTORY_X 10
#define INVENTORY_Y 40
#define SLOT_SIZE 36
#define LOG_LINES 6
#define LOG_HISTORY 100
#define LOG_LINE_CHARS 38
#define LOG_LINE_HEIGHT 12
#define LOG_WIDTH 238
#define LOG_BUTTON_SPACING 20


enum GameWorldScene {
    connection = 0,
    connection_loading = 1,
    connection_error = 2,
    in_game = 3,
};

class GameWorld : public GameScene {
    public:
        int fd;

        float alive_last_time;
        float alive_timer;
        float timer;
        int server_players;
        bool wait_quests;
        int log_scroll;

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
        std::map<std::string, Texture2D> object_textures;
        std::string hovered_item;
        std::string hovered_slot;
        std::vector<std::string> log;

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
        void draw_player_count();
        void draw_world_objects();
        void draw_object_row(std::vector<std::string> &ids, std::string prefix, float row_offset, Vector2 mouse);
        Texture2D get_object_texture(const std::string &file);
        std::string display_name(const std::string &id);
        void draw_inventory();
        void add_log(std::string msg);
        void draw_log();
        void update_server_players(std::string msg);
        void update_items(std::string msg);
        void draw_island(std::vector<std::vector<int>> &island);
};
