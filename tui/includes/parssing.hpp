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

struct QuestInfo {
    std::string quest_id;
    std::string description;
    std::string reward;
    std::string status;
    std::string progress;
};

bool parseLookResponse(const std::string& raw_response, RoomInfo& out);
bool parseQuestResponse(const std::string& raw_response, QuestInfo& out);
bool parseQuestsResponse(const std::string& raw_response,
                         std::vector<QuestInfo>& out);
bool parseInventoryResponse(const std::string& raw_response,
                            std::vector<std::string>& out);
bool parseGenericOkJson(const std::string& raw_response,
                        std::vector<std::string>& out_lines);

std::vector<std::string> formatRoomInfo(const RoomInfo& info);
std::vector<std::string> formatQuestInfo(const QuestInfo& info);
