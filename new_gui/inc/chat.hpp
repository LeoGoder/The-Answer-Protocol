#pragma once

#include "raylib.h"
#include <string>
#include <vector>

#define MAX_MSG 5
#define MAX_MSG_LENGTH 30

#define CHAT_WIDTH int(SCREEN_WIDTH * 0.80)
#define CHAT_HEIGHT int(SCREEN_HEIGHT * 0.80)
#define CHAT_MARGIN_X int(SCREEN_WIDTH / 2) - int(CHAT_WIDTH / 2)
#define CHAT_MARGIN_Y int(SCREEN_HEIGHT / 2) - int(CHAT_HEIGHT / 2) - 35
#define CHAT_INPUT_MARGIN 30

enum ChatService {
    global = 1,
    room = 2,
    group = 3,
};

class Chat {
    private:
        std::vector<std::string> chat_global;
        std::string msg;

    public:
        ChatService chat;
        Texture2D *font;

        Chat(Texture2D *font);
        ~Chat();

        void draw_chat();
        void draw_msg();
        void chat_input(int fd);
        void add_global_msg(std::string msg);
};

