/**
 * @file protocol.hpp
 * @brief TAP protocol message parsing, payload deserialization, and presentation formatting.
 */

#pragma once

#include <string>
#include <vector>

#include "../includes/models.hpp"

/**
 * @brief Parses the JSON payload from a 'OK { room... }' response into structured RoomInfo.
 *
 * Extracts room id, name, description, cardinal exits dictionary, and arrays of
 * visible players, items, and NPCs.
 *
 * @param raw_response Full server response line starting with "OK ".
 * @param out Output RoomInfo populated on successful parse.
 * @return True on success, false if JSON syntax is invalid or prefix is missing.
 */
bool parseLookResponse(const std::string& raw_response, RoomInfo& out);

// Parses single 'OK { quest... }' JSON response into QuestInfo
bool parseQuestResponse(const std::string& raw_response, QuestInfo& out);

// Parses 'OK [ { quest... }, ... ]' JSON array into vector of active quests
bool parseQuestsResponse(const std::string& raw_response,
                         std::vector<QuestInfo>& out);

// Parses 'OK [ "item1", ... ]' JSON string array into player inventory vector
bool parseInventoryResponse(const std::string& raw_response,
                            std::vector<std::string>& out);

// Parses 'OK { "used": ... }' item consumption result into UseInfo
bool parseUseResponse(const std::string& raw_response, UseInfo& out);

/**
 * @brief Parses ATTACK command response or EVT ROOM COMBAT broadcast into CombatInfo.
 *
 * Handles both "OK { ... }" synchronous attack replies and "EVT ROOM COMBAT { ... }"
 * asynchronous room event notifications.
 *
 * @param raw_response Server message string containing combat JSON.
 * @param out Output CombatInfo structure.
 * @return True if combat payload was parsed successfully, false otherwise.
 */
bool parseCombatResponse(const std::string& raw_response, CombatInfo& out);

/**
 * @brief Parses arbitrary unknown JSON payloads and flattens them into human-readable text lines.
 *
 * Recursively traverses nested JSON structures to provide a generic fallback display
 * for undocumented or extension protocol messages.
 *
 * @param raw_response Server response line starting with "OK ".
 * @param out_lines Output list of formatted strings.
 * @return True if valid JSON was flattened, false otherwise.
 */
bool parseGenericOkJson(const std::string& raw_response,
                        std::vector<std::string>& out_lines);

// Formats RoomInfo fields into human-readable terminal lines
std::vector<std::string> formatRoomInfo(const RoomInfo& info);

// Formats QuestInfo fields into human-readable terminal lines
std::vector<std::string> formatQuestInfo(const QuestInfo& info);

// Formats item consumption results (HP changes, attack boosts) into display lines
std::vector<std::string> formatUseInfo(const UseInfo& info);

// Formats combat round outcome and damage details into display lines
std::vector<std::string> formatCombatInfo(const CombatInfo& info);

/**
 * @brief Parses user input to identify CHAT commands, target channels, and format wire frames.
 *
 * Supports channels like ROOM, GLOBAL, and PARTY/GROUP. Validates command syntax
 * and prepares the normalized "CHAT <SCOPE> <MESSAGE>" protocol string.
 *
 * @param input Raw command line entered by user.
 * @return ParsedChat structure indicating if input is a chat action and formatted wire payload.
 */
ParsedChat parseChatCommand(const std::string& input);
