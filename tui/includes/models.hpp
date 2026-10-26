#pragma once

#include <map>
#include <string>
#include <vector>

struct ConnectionConfig {
    std::string pseudo;
    std::string ip   = "127.0.0.1";
    std::string port = "4242";
};
using InfosConnexion = ConnectionConfig;

struct ChatMessage {
    std::string pseudo;
    std::string content;
    std::string channel;
    bool is_local = false;
};

struct RoomInfo {
    std::string id;
    std::string name;
    std::string description;
    std::map<std::string, std::string> exits;
    std::vector<std::string> players;
    std::vector<std::string> items;
    std::vector<std::string> npcs;
};

struct QuestInfo {
    std::string quest_id;
    std::string description;
    std::string reward;
    std::string status;
    std::string progress;
    std::string objective;
    int required_items = 0;
    std::string target_item;
    std::string target_npc;
};

struct UseInfo {
    std::string used;
    int hp = 0;
    int max_hp = 0;
    int attack = 0;
    std::string received;
};

struct CombatInfo {
    std::string attacker;
    std::string target;
    int attacker_hp = 0;
    int target_hp = 0;
    int damage = 0;
    int damage_taken = 0;
    std::string status;
    std::string room;
};

enum PendingCommand {
    CMD_NONE,
    CMD_LOOK,
    CMD_QUEST,
    CMD_QUESTS,
    CMD_INVENTORY,
    CMD_INVENTORY_AUTO,
    CMD_TAKE,
    CMD_DROP,
    CMD_MOVE,
    CMD_OTHER_JSON,
    CMD_USE,
    CMD_ATTACK
};

struct PendingChat {
    std::string pseudo;
    std::string content;
    std::string channel;
    std::string raw_command;
};

struct ParsedChat {
    bool is_chat = false;
    std::string channel;
    std::string message;
    std::string command_to_send;
};
