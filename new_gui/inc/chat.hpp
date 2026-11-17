#pragma once

#include <string>

#define MAX_MSG_LENGTH 100

enum ChatService {
    global = 0,
    room = 1,
    group = 2,
};

class Chat {
 private:
        std::string msg;

 public:
        ChatService chat;

        Chat();
        ~Chat();

        std::string get_msg() const {
            return this->msg;
        }
        void clear_msg() {
            this->msg.clear();
        }
        bool chat_input();
};
