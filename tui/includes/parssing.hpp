#pragma once

#include <map>
#include <string>
#include <vector>

struct RoomInfo {
    std::string id;
    std::string name;
    std::string description;
    std::map<std::string, std::string> exits;
    std::vector<std::string> players;
    std::vector<std::string> items;
    std::vector<std::string> npcs;
};

bool parseLookResponse(const std::string& raw_response, RoomInfo& out);

std::vector<std::string> formatRoomInfo(const RoomInfo& info);
