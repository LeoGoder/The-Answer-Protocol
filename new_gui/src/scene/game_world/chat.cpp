#include "chat.hpp"
#include "helper.hpp"
#include "raylib.h"
#include "server.hpp"
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

Chat::Chat(Texture2D *font) {
    this->chat = global;
    this->font = font;
}

Chat::~Chat() {

}

void Chat::draw_chat() {
    Rectangle msg_view = {CHAT_MARGIN_X, CHAT_MARGIN_Y, CHAT_WIDTH, CHAT_HEIGHT};
    Rectangle my_msg = {CHAT_MARGIN_X, CHAT_HEIGHT + CHAT_INPUT_MARGIN, CHAT_WIDTH, 60};

    DrawRectangle(msg_view.x, msg_view.y, msg_view.width, msg_view.height, ColorAlpha(BLACK, 0.70));
    DrawRectangle(my_msg.x, my_msg.y, my_msg.width, my_msg.height, ColorAlpha(BLACK, 0.70));
}

void Chat::draw_msg() {
    const float x = CHAT_MARGIN_X;
    float y = CHAT_MARGIN_Y;

    for (size_t i = 0; i < this->chat_global.size(); i++) {
        DrawFontText(*this->font, this->chat_global[i], x, y, WHITE, 1.0f);

        y += CHAR_HEIGHT;
    }
    DrawFontText(*this->font, this->msg, x, CHAT_HEIGHT + CHAT_INPUT_MARGIN);
}

void Chat::chat_input(int fd) {
    int key = GetKeyPressed();

    if (key == KEY_SLASH) {
        // do nothing
    }
    else if (key == KEY_BACKSPACE) {
        if (!this->msg.empty())
            this->msg.pop_back();
    }
    else if (key == KEY_ENTER) {
        std::string final_msg = "CHAT GLOBAL " + this->msg + '\n';

        send_message(final_msg, "", fd);
        this->msg.clear();
    }
    else if (key > 0 && this->msg.size() < MAX_MSG_LENGTH) {
        if (key == KEY_SPACE)
            this->msg.push_back(key);
        else
            this->msg.push_back(key + 32);
    }
}

void Chat::add_global_msg(std::string msg) {
    std::vector<std::string> parts = split(msg, ' ', 1);
    std::string final_msg = parts[0] + ':' + parts[1];

    this->chat_global.push_back(final_msg);
}

