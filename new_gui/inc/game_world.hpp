#pragma once

#include <netinet/in.h>

#include <map>
#include <string>
#include <vector>

#include "inc/game_scene.hpp"
#include "inc/game_state.hpp"
#include "inc/map_parse.hpp"
#include "inc/player.hpp"
#include "inc/raylib.h"
#include "inc/server.hpp"

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
#define LOG_LINE_CHARS 81
#define LOG_LINE_HEIGHT 12
#define LOG_WIDTH 500
#define PANEL_LOG 0
#define PANEL_GROUP 1
#define PANEL_CHAT 2
#define BUTTON_SPACING 6
#define BUTTON_PADDING 3
#define BUTTON_HEIGHT (HUD_FONT_SIZE + BUTTON_PADDING * 2)
#define DIALOG_WIDTH 320
#define DIALOG_HEIGHT 80
#define DIALOG_LINE_CHARS 52
#define MINIMAP_CELL 14
#define MINIMAP_STEP 26
#define MINIMAP_SIZE (2 * MINIMAP_STEP + MINIMAP_CELL + HUD_PADDING * 2)
#define MINIMAP_HEIGHT (MINIMAP_SIZE + LOG_LINE_HEIGHT)
#define QUESTS_WIDTH 150
#define REQUEST_FREQUENCE 1.0 / 24.0


enum GameWorldScene {
    connection = 0,
    connection_loading = 1,
    connection_error = 2,
    in_game = 3,
};

class GameWorld : public GameScene {
 public:
        int fd;
        std::string pending;

        float alive_last_time;
        float alive_timer;
        float timer;
        int server_players;
        int hp;
        int max_hp;
        bool wait_quests;
        bool log_quests;
        bool wait_talk;
        int log_scroll;
        int panel_tab;

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
        std::vector<std::string> panel_lines[5];
        std::string group_inviter;
        std::vector<std::string> group_members;
        bool group_unread;
        bool chat_unread[3];
        bool inviting;
        std::vector<std::string> hostile;
        std::vector<std::string> quest_givers;
        std::vector<std::string> usable;
        std::vector<std::string> quests;
        std::string hovered_npc;
        std::string hovered_exit;
        std::string dialog_npc;
        std::string dialog_text;

        explicit GameWorld(GameState *game_state);
        ~GameWorld();

        void update() override;
        void server_alive(std::string msg);
        void net_update();
        void room_presence_enter(std::string new_player);
        void room_presence_leave(std::string delete_player);
        void draw() override;
        void draw_loading_screen();
        void draw_connection_error();
        void draw_game();
        void draw_hud();
        void draw_player_count(int x, int y, int width);
        void draw_player_info();
        void draw_minimap();
        void draw_quests();
        void refresh_quests(std::string msg);
        void draw_world_objects();
        Vector2 world_mouse();
        void draw_exits(std::vector<std::vector<int>> &island);
        void draw_object_row(std::vector<std::string> &ids, std::string prefix,
                             float row_offset, Vector2 mouse);
        Texture2D get_object_texture(const std::string &file);
        std::string display_name(const std::string &id);
        void draw_inventory();
        void add_log(std::string msg, int tab = PANEL_LOG);
        void draw_group_buttons(int x, int y);
        void draw_chat_buttons(int x, int y);
        void draw_bubble(float x, float y);
        void open_chat();
        void send_chat();
        void read_chat(std::string buffer);
        void update_group(std::string msg);
        void draw_log();
        int draw_text_buttons(std::vector<std::string> names, int x, int y,
                              int selected = -1);
        Rectangle dialog_rect();
        void draw_dialog();
        void read_talk_reply(std::string buffer);
        void update_server_players(std::string msg);
        void update_hp(std::string msg);
        void update_items(std::string msg);
        void draw_island(std::vector<std::vector<int>> &island);
};
