#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "inc/player.hpp"

#define MAX_DATA 1024

struct NetCommand {
    std::string CONNECT = "CONNECT ";
    std::string LOOK = "LOOK";
    std::string ALIVE = "ALIVE";
    std::string MOVE = "MOVE";
    std::string WHO = "WHO";
    std::string INVENTORY = "INVENTORY";
    std::string TAKE = "TAKE ";
    std::string DROP = "DROP ";
    std::string USE = "USE ";
    std::string STATUS = "STATUS";
    std::string QUESTS = "QUESTS";
    std::string WORLD = "WORLD";
    std::string TALK = "TALK ";
    std::string QUEST = "QUEST ";
    std::string ATTACK = "ATTACK ";
    std::string GROUP = "GROUP ";
    std::string CHAT = "CHAT ";
    std::string QUIT = "QUIT";
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
