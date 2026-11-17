#include "inc/chat.hpp"

#include <string>

#include "inc/raylib.h"

Chat::Chat() {
    this->chat = global;
}

Chat::~Chat() {

}

// Reads the typed characters; returns true on Enter (typing is over).
bool Chat::chat_input() {
    int key = GetCharPressed();

    while (key > 0) {
        if (key >= 32 && key < 127 && this->msg.size() < MAX_MSG_LENGTH)
            this->msg.push_back(key);
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !this->msg.empty())
        this->msg.pop_back();
    return IsKeyPressed(KEY_ENTER);
}
