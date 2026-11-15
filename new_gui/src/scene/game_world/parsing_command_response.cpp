#include "helper.hpp"
#include "map_parse.hpp"
#include "player.hpp"
#include "raylib.h"
#include "server.hpp"
#include <cstddef>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

void parse_look_response(std::vector<std::string> &response, Room &room, Texture2D *font, Texture2D *sprite, Maps &map) {
    std::vector<std::string> response_split;
    std::string exits = extract_sub_tab(response[1], "exits");
    std::vector<std::string> players_extract = extract_vector(response[1], "players");
    std::vector<std::string> items_extract = extract_vector(response[1], "items");
    std::vector<std::string> npcs_extract = extract_vector(response[1], "npcs");

    room.player.clear();

    room.exists.east = extract_value(response[1], "east");
    room.exists.north = extract_value(response[1], "north");
    room.exists.south = extract_value(response[1], "south");
    room.exists.west = extract_value(response[1], "west");

    room.id = extract_value(response[1], "id");
    room.description = extract_value(response[1], "description");
    room.name = extract_value(response[1], "name");
    room.items = items_extract;
    room.npcs = npcs_extract;
    std::string names_tab = extract_sub_tab(response[1], "names");
    size_t key_start = names_tab.find('"');

    while (key_start != std::string::npos) {
        size_t key_end = names_tab.find('"', key_start + 1);
        size_t value_start = names_tab.find('"', key_end + 1);
        size_t value_end = names_tab.find('"', value_start + 1);

        if (value_end == std::string::npos)
            break;
        std::string key = names_tab.substr(key_start + 1, key_end - key_start - 1);

        room.names[key] = names_tab.substr(value_start + 1, value_end - value_start - 1);
        key_start = names_tab.find('"', value_end + 1);
    }
    for (size_t i = 0; i < players_extract.size(); i++) {
        room.player.push_back(std::make_unique<Player>(font, players_extract[i], sprite));
    }
    
    if (room.id == "hub")
        map = hub;
    if (room.id == "komolexos")
        map = komolexos;
    if (room.id == "jeetomal")
        map = jeetomal;
    if (room.id == "smycka")
        map = smycka;
    if (room.id == "vinterfint")
        map = vinterfint;
    if (room.id == "fejka")
        map = fejka;
    if (room.id == "kuggis")
        map = kuggis;
    if (room.id == "fugoumitak")
        map = fugoumitak;
}

void parse_move_response(std::string &player_name, Room &room, NetCommand &command, int fd) {
    (void) room;
    (void) player_name;

    send_message(command.LOOK, "", fd);
}

std::vector<std::string> parse_id_list(std::string &msg, std::string key) {
    std::vector<std::string> inventory;
    size_t start = msg.find(key);
    size_t end = msg.find(']', start);
    std::string list = msg.substr(start + key.size(), end - start - key.size());
    size_t cursor = list.find('"');

    while (cursor != std::string::npos) {
        size_t close = list.find('"', cursor + 1);

        if (close == std::string::npos)
            break;
        inventory.push_back(list.substr(cursor + 1, close - cursor - 1));
        cursor = list.find('"', close + 1);
    }
    return inventory;
}