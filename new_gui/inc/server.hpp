#pragma once

#include "player.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

#define MAX_DATA 1024

struct NetCommand {
    std::string CONNECT = "CONNECT ";
    std::string LOOK = "LOOK\n";
    std::string ALIVE = "ALIVE\n";
    std::string MOVE = "MOVE\n";
    std::string WHO = "WHO\n";
    std::string INVENTORY = "INVENTORY\n";
    std::string TAKE = "TAKE ";
    std::string DROP = "DROP ";
    std::string USE = "USE ";
    std::string STATUS = "STATUS\n";
    std::string QUESTS = "QUESTS\n";
    std::string WORLD = "WORLD\n";
    std::string TALK = "TALK ";
    std::string QUEST = "QUEST ";
    std::string ATTACK = "ATTACK ";
    std::string POSITION = "POSITION ";
};

struct EvtCommand {
    std::string ROOM_ENTER = "ROOM PRESENCE ENTER ";
    std::string ROOM_LEAVE = "ROOM PRESENCE LEAVE ";
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
    std::map<std::string, std::string> names;
};

int send_message(std::string command, std::string param, int fd);
std::string receiv_msg(int fd);
