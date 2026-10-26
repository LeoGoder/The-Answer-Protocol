#pragma once

#include <string>
#include <vector>

#include "../includes/models.hpp"

bool parseLookResponse(const std::string& raw_response, RoomInfo& out);
bool parseQuestResponse(const std::string& raw_response, QuestInfo& out);
bool parseQuestsResponse(const std::string& raw_response,
                         std::vector<QuestInfo>& out);
bool parseInventoryResponse(const std::string& raw_response,
                            std::vector<std::string>& out);
bool parseUseResponse(const std::string& raw_response, UseInfo& out);
bool parseCombatResponse(const std::string& raw_response, CombatInfo& out);
bool parseGenericOkJson(const std::string& raw_response,
                        std::vector<std::string>& out_lines);

std::vector<std::string> formatRoomInfo(const RoomInfo& info);
std::vector<std::string> formatQuestInfo(const QuestInfo& info);
std::vector<std::string> formatUseInfo(const UseInfo& info);
std::vector<std::string> formatCombatInfo(const CombatInfo& info);

ParsedChat parseChatCommand(const std::string& input);
