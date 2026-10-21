#include "helper.hpp"
#include "server.hpp"
#include <string>
#include <vector>

void parse_look_response(std::vector<std::string> &response, Room &room) {
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
    room.player = players_extract;
    room.items = items_extract;
    room.npcs = npcs_extract;
}
