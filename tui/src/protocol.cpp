// TAP protocol parsing, deserialization, and presentation formatting

#include "../includes/protocol.hpp"

#include <cctype>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "../includes/ui_helpers.hpp"
#include "../lib/json.hpp"

using json = nlohmann::json;

// --- Helper Functions for String & Room Display ---

// Formats vector of strings into comma-separated capitalized list
static std::string joinList(const std::vector<std::string>& v) {
    if (v.empty()) return "None";
    std::string result;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i > 0) result += ", ";
        std::string word = v[i];
        if (!word.empty()) {
            word[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(word[0])));
        }
        result += word;
    }
    return result;
}

// Strips namespace prefix from room ID and capitalizes name
static std::string roomIdToName(const std::string& room_id) {
    std::string name = room_id;
    size_t dot = name.find('.');
    if (dot != std::string::npos) name = name.substr(dot + 1);
    if (!name.empty()) {
        name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
    }
    return name;
}

// Formats cardinal exits map into readable links
static std::string joinExits(const std::map<std::string, std::string>& exits) {
    if (exits.empty()) return "None";
    std::string result;
    bool first = true;
    for (const auto& [dir, room_id] : exits) {
        if (!first) result += ", ";
        result += roomIdToName(room_id) + " (" + dir + ")";
        first = false;
    }
    return result;
}

// Extracts the first sequence of digits found in a string
static int extractFirstNumber(const std::string& text) {
    for (size_t i = 0; i < text.size(); ++i) {
        if (std::isdigit(static_cast<unsigned char>(text[i]))) {
            size_t j = i;
            while (j < text.size() && std::isdigit(static_cast<unsigned char>(text[j]))) {
                j++;
            }
            try {
                return std::stoi(text.substr(i, j - i));
            } catch (...) {
                return 0;
            }
        }
    }
    return 0;
}

// Extracts targets, item counts, and recipient NPCs from quest description
static void parseQuestObjective(QuestInfo& out) {
    if (out.description.empty()) return;
    std::string text = out.description;
    size_t colon = text.find(':');
    if (colon != std::string::npos) {
        if (out.quest_id.empty()) {
            out.quest_id = trim(text.substr(0, colon));
        }
        text = text.substr(colon + 1);
    }
    out.objective = trim(text);

    std::string lower = out.objective;
    for (char& c : lower) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    size_t bring = lower.find("bring ");
    size_t to    = lower.find(" to ");
    if (bring != std::string::npos && to != std::string::npos && to > bring + 6) {
        out.target_item = trim(out.objective.substr(bring + 6, to - (bring + 6)));
        std::string npc = trim(out.objective.substr(to + 4));
        while (!npc.empty() && (npc.back() == '.' || std::isspace(static_cast<unsigned char>(npc.back())))) {
            npc.pop_back();
        }
        out.target_npc = npc;
        int count = extractFirstNumber(out.target_item);
        out.required_items = (count > 0) ? count : 1;
    } else {
        out.required_items = extractFirstNumber(out.objective);
        out.target_item = out.objective;
    }

    if (out.progress.empty() && out.required_items > 0) {
        out.progress = std::to_string(out.required_items) + " required";
    }
}

// --- JSON Deserializers (ADL from_json) ---

void from_json(const json& j, RoomInfo& out) {
    if (j.contains("room") && j["room"].is_object()) {
        const auto& room = j["room"];
        out.id          = room.value("id", "");
        out.name        = room.value("name", "");
        out.description = room.value("description", "");
        if (room.contains("exits") && room["exits"].is_object()) {
            out.exits = room["exits"].get<std::map<std::string, std::string>>();
        }
    }
    out.players = j.value("players", std::vector<std::string>{});
    out.items   = j.value("items", std::vector<std::string>{});
    out.npcs    = j.value("npcs", std::vector<std::string>{});
}

void from_json(const json& j, QuestInfo& out) {
    if (j.contains("quest_id") && j["quest_id"].is_string()) {
        out.quest_id = j["quest_id"].get<std::string>();
    } else if (j.contains("id")) {
        if (j["id"].is_string()) {
            out.quest_id = j["id"].get<std::string>();
        } else if (j["id"].is_number()) {
            out.quest_id = std::to_string(j["id"].get<int>());
        }
    }
    out.description = j.value("description", "");
    out.reward      = j.value("reward", "");
    out.status      = j.value("status", "");
    out.progress    = j.value("progress", "");
    parseQuestObjective(out);
}

void from_json(const json& j, UseInfo& out) {
    out.used     = j.value("used", "");
    out.hp       = j.value("hp", 0);
    out.max_hp   = j.value("max_hp", 0);
    out.attack   = j.value("attack", 0);
    out.received = j.value("received", "");
}

void from_json(const json& j, CombatInfo& out) {
    out.attacker     = j.value("attacker", "");
    out.target       = j.value("target", "");
    out.attacker_hp  = j.value("attacker_hp", 0);
    out.target_hp    = j.value("target_hp", 0);
    out.damage       = j.value("damage", 0);
    out.damage_taken = j.value("damage_taken", 0);
    out.status       = j.value("status", "");
    out.room         = j.value("room", "");
}

// --- Protocol Response Parsers ---

bool parseLookResponse(const std::string& raw_response, RoomInfo& out) {
    std::string payload;
    if (!stripPrefix(raw_response, "OK ", payload)) return false;
    try {
        json j = json::parse(payload);
        if (!j.is_object()) return false;
        out = j.get<RoomInfo>();
        return true;
    } catch (...) {
        return false;
    }
}

bool parseQuestResponse(const std::string& raw_response, QuestInfo& out) {
    std::string payload;
    if (!stripPrefix(raw_response, "OK ", payload)) return false;
    try {
        json j = json::parse(payload);
        if (!j.is_object()) return false;
        out = j.get<QuestInfo>();
        return true;
    } catch (...) {
        return false;
    }
}

bool parseQuestsResponse(const std::string& raw_response,
                         std::vector<QuestInfo>& out) {
    std::string payload;
    if (!stripPrefix(raw_response, "OK ", payload)) return false;
    try {
        json j = json::parse(payload);
        if (!j.is_array()) return false;
        out = j.get<std::vector<QuestInfo>>();
        return true;
    } catch (...) {
        return false;
    }
}

bool parseInventoryResponse(const std::string& raw_response,
                            std::vector<std::string>& out) {
    std::string payload;
    if (!stripPrefix(raw_response, "OK ", payload)) return false;
    try {
        json j = json::parse(payload);
        if (!j.is_array()) return false;
        for (const auto& item : j) {
            if (item.is_string()) {
                out.push_back(item.get<std::string>());
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool parseUseResponse(const std::string& raw_response, UseInfo& out) {
    std::string payload;
    if (!stripPrefix(raw_response, "OK ", payload)) return false;
    try {
        json j = json::parse(payload);
        if (!j.is_object() || !j.contains("used") || !j["used"].is_string()) {
            return false;
        }
        out = j.get<UseInfo>();
        return true;
    } catch (...) {
        return false;
    }
}

bool parseCombatResponse(const std::string& raw_response, CombatInfo& out) {
    std::string payload;
    if (!stripPrefix(raw_response, "OK ", payload) &&
        !stripPrefix(raw_response, "EVT ROOM COMBAT ", payload)) {
        return false;
    }
    try {
        json j = json::parse(payload);
        if (!j.is_object()) return false;
        out = j.get<CombatInfo>();
        return true;
    } catch (...) {
        return false;
    }
}

// Recursively traverses arbitrary nested JSON objects and arrays into text lines
static void formatJsonValue(const json& j, const std::string& prefix,
                            std::vector<std::string>& lines) {
    if (j.is_object()) {
        for (auto& [key, val] : j.items()) {
            std::string label = key;
            if (!label.empty()) {
                label[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(label[0])));
            }
            if (val.is_object() || val.is_array()) {
                formatJsonValue(val, prefix + label + ".", lines);
            } else if (val.is_string()) {
                lines.push_back(prefix + label + ": " + val.get<std::string>());
            } else {
                lines.push_back(prefix + label + ": " + val.dump());
            }
        }
    } else if (j.is_array()) {
        for (size_t i = 0; i < j.size(); ++i) {
            if (j[i].is_string()) {
                lines.push_back(prefix + j[i].get<std::string>());
            } else if (j[i].is_object() || j[i].is_array()) {
                formatJsonValue(j[i], prefix, lines);
            } else {
                lines.push_back(prefix + j[i].dump());
            }
        }
    } else if (j.is_string()) {
        lines.push_back(prefix + j.get<std::string>());
    } else {
        lines.push_back(prefix + j.dump());
    }
}

bool parseGenericOkJson(const std::string& raw_response,
                        std::vector<std::string>& out_lines) {
    std::string payload;
    if (!stripPrefix(raw_response, "OK ", payload)) return false;
    try {
        json j = json::parse(payload);
        formatJsonValue(j, "", out_lines);
        return true;
    } catch (...) {
        return false;
    }
}

// --- Presentation Formatters ---

std::vector<std::string> formatRoomInfo(const RoomInfo& info) {
    std::vector<std::string> lines;
    lines.push_back("Room: " + info.name);
    lines.push_back("Description: " + info.description);
    lines.push_back("Exits: " + joinExits(info.exits));
    lines.push_back("Items: " + joinList(info.items));
    lines.push_back("Players: " + joinList(info.players));
    lines.push_back("NPCs: " + joinList(info.npcs));
    return lines;
}

std::vector<std::string> formatQuestInfo(const QuestInfo& info) {
    std::vector<std::string> lines;
    if (!info.quest_id.empty()) lines.push_back("Quest: " + info.quest_id);
    if (!info.description.empty()) lines.push_back("Description: " + info.description);
    if (!info.objective.empty() && info.objective != info.description) {
        lines.push_back("Objective: " + info.objective);
    }
    if (info.required_items > 0) {
        lines.push_back("Required items: " + std::to_string(info.required_items));
    }
    if (!info.target_item.empty()) lines.push_back("Target item: " + info.target_item);
    if (!info.target_npc.empty()) lines.push_back("Deliver to: " + info.target_npc);
    if (!info.progress.empty()) lines.push_back("Progress: " + info.progress);
    if (!info.reward.empty()) lines.push_back("Reward: " + info.reward);
    if (!info.status.empty()) lines.push_back("Status: " + info.status);
    return lines;
}

std::vector<std::string> formatUseInfo(const UseInfo& info) {
    std::vector<std::string> lines;
    std::string item_name = info.used;
    if (!item_name.empty()) {
        item_name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(item_name[0])));
    }
    lines.push_back("Used: " + item_name);

    if (info.max_hp > 0) {
        lines.push_back("HP: " + std::to_string(info.hp) + "/" + std::to_string(info.max_hp));
    } else if (info.hp > 0) {
        lines.push_back("HP: " + std::to_string(info.hp));
    }

    if (info.attack > 0) {
        lines.push_back("Attack: " + std::to_string(info.attack));
    }

    if (!info.received.empty()) {
        std::string rec = info.received;
        if (!rec.empty()) {
            rec[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(rec[0])));
        }
        lines.push_back("Received: " + rec);
    }

    return lines;
}

std::vector<std::string> formatCombatInfo(const CombatInfo& info) {
    std::vector<std::string> lines;
    std::string target_name = info.target;
    if (!target_name.empty()) {
        target_name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(target_name[0])));
    }

    lines.push_back("Combat vs " + target_name + ":");
    lines.push_back("  Damage dealt: " + std::to_string(info.damage) +
                    " (" + target_name + " HP: " + std::to_string(info.target_hp) + ")");

    if (info.status == "victory") {
        lines.push_back("  Status: Victory! " + target_name + " defeated.");
    } else {
        lines.push_back("  Damage taken: " + std::to_string(info.damage_taken) +
                        " (Your HP: " + std::to_string(info.attacker_hp) + ")");
        if (info.status == "defeated") {
            std::string respawn = info.room.empty() ? "respawn point" : info.room;
            lines.push_back("  Status: Defeated! You respawned in " + respawn + ".");
        } else {
            lines.push_back("  Status: In combat");
        }
    }

    return lines;
}

// Parses user input to identify CHAT commands and normalize wire payload
ParsedChat parseChatCommand(const std::string& input) {
    ParsedChat res;
    std::istringstream iss(trim(input));
    std::string verb, scope;
    if (!(iss >> verb >> scope) || toUpper(verb) != "CHAT") return res;

    std::string up_scope = toUpper(scope);
    std::string wire_scope;
    if (up_scope == "ROOM") {
        res.channel = "ROOM";
        wire_scope = "ROOM";
    } else if (up_scope == "GLOBAL") {
        res.channel = "GLOBAL";
        wire_scope = "GLOBAL";
    } else if (up_scope == "PARTY" || up_scope == "GROUP") {
        res.channel = "PARTY";
        wire_scope = "GROUP";
    } else {
        return res;
    }

    std::string msg;
    std::getline(iss >> std::ws, msg);
    res.message = trim(msg);
    if (res.message.empty()) return res;

    res.is_chat = true;
    res.command_to_send = "CHAT " + wire_scope + " " + res.message;
    return res;
}
