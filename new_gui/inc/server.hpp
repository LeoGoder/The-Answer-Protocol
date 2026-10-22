#pragma once
#include "player.hpp"
#include <memory>
#include <string>
#include <vector>
#define MAX_DATA 1024

struct NetCommand {
    std::string CONNECT = "CONNECT ";
    std::string LOOK = "LOOK\n";
    std::string ALIVE = "ALIVE\n";
};

struct Exits {
    std::string north;
    std::string east;
    std::string south;
    std::string west;
};

struct Room {
    std::string id;
    std::string name;
    std::string description;
    Exits exists;
    std::vector<std::unique_ptr<Player>> player;
    std::vector<std::string> items;
    std::vector<std::string> npcs;
};

int send_message(std::string command, std::string param, int fd);
std::string receiv_msg(int fd);
