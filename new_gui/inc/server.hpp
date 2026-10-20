#pragma once
#include <string>
#define MAX_DATA 1024

struct NetCommand {
    std::string CONNECT = "CONNECT ";
    std::string LOOK = "LOOK\n";
    std::string ALIVE = "ALIVE\n";
};

int send_message(std::string command, std::string param, int fd);
std::string receiv_msg(int fd);
