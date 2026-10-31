#include "helper.hpp"
#include "map_parse.hpp"
#include "player.hpp"
#include "raylib.h"
#include "server.hpp"
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

void parse_look_response(std::vector<std::string> &response, Room &room, Texture2D *font, Texture2D *sprite, Maps &map) {
    std::vector<std::string> response_split;
    std::string exits = extract_sub_tab(response[1], "exits");
    std::vector<std::string> players_extract = extract_vector(response[1], "players");
    std::vector<std::string> items_extract = extract_vector(response[1], "items");
    std::vector<std::string> npcs_extract = extract_vector(response[1], "npcs");

    room.exists.east = extract_value(response[1], "east");
    room.exists.north = extract_value(response[1], "north");
    room.exists.south = extract_value(response[1], "south");
    room.exists.west = extract_value(response[1], "west");

    room.id = extract_value(response[1], "id");
    room.description = extract_value(response[1], "description");
    room.name = extract_value(response[1], "name");
    room.items = items_extract;
    room.npcs = npcs_extract;
    for (size_t i = 0; i < players_extract.size(); i++) {
        room.player.push_back(std::make_unique<Player>(font, players_extract[i], sprite));
    }
    
    if (room.id == "room7")
        map = fugoumitak;
    if (room.id == "room6")
        map = kuggis;
    if (room.id == "room5")
        map = fejka;
    if (room.id == "room4")
        map = vinterfint;
    if (room.id == "room3")
        map = smycka;
    if (room.id == "room2")
        map = jeetomal;
    if (room.id == "room1")
        map = hub;
}

void parse_move_response(std::vector<std::string> &response, Room &room, NetCommand &command, int fd) {
    (void) response;
    (void) room;
    (void) command;
    (void) fd;
    // faire un check si la room existe bien
    std::cout << "JE MOOOOOOOOOOOOOOOOOVE\n";
    send_message(command.LOOK, "", fd);
}
