#include "../includes/protocol.hpp"

#include <cctype>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../lib/json.hpp"

using json = nlohmann::json;

bool parseLookResponse(const std::string& raw_response, RoomInfo& out) {
    const std::string prefix = "OK ";
    if (raw_response.substr(0, prefix.size()) != prefix) {
        return false;
    }

    std::string json_str = raw_response.substr(prefix.size());

    try {
        json j = json::parse(json_str);

        if (j.contains("room") && j["room"].is_object()) {
            const auto& room = j["room"];
            if (room.contains("id") && room["id"].is_string())
                out.id = room["id"].get<std::string>();
            if (room.contains("name") && room["name"].is_string())
                out.name = room["name"].get<std::string>();
            if (room.contains(
                    "description") && room["description"].is_string())
                out.description = room["description"].get<std::string>();
            if (room.contains("exits") && room["exits"].is_object()) {
                for (auto& [dir, target] : room["exits"].items()) {
                    if (target.is_string())
                        out.exits[dir] = target.get<std::string>();
                }
            }
        }

        if (j.contains("players") && j["players"].is_array()) {
            for (const auto& p : j["players"]) {
                if (p.is_string())
                    out.players.push_back(p.get<std::string>());
            }
        }

        if (j.contains("items") && j["items"].is_array()) {
            for (const auto& it : j["items"]) {
                if (it.is_string())
                    out.items.push_back(it.get<std::string>());
            }
        }

        if (j.contains("npcs") && j["npcs"].is_array()) {
            for (const auto& npc : j["npcs"]) {
                if (npc.is_string())
                    out.npcs.push_back(npc.get<std::string>());
            }
        }

        return true;
    } catch (const json::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
}

static std::string joinList(const std::vector<std::string>& v) {
    if (v.empty()) return "None";
    std::string result;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i > 0) result += ", ";
        std::string word = v[i];
        if (!word.empty())
            word[0] = static_cast<char>(std::toupper(word[0]));
        result += word;
    }
    return result;
}

static std::string roomIdToName(const std::string& room_id) {
    std::string name = room_id;
    size_t dot = name.find('.');
    if (dot != std::string::npos)
        name = name.substr(dot + 1);
    if (!name.empty())
        name[0] = static_cast<char>(std::toupper(name[0]));
    return name;
}

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

static void parseQuestObjective(QuestInfo& out) {
    if (out.description.empty()) return;

    size_t colon_pos = out.description.find(':');
    std::string after_colon;
    if (colon_pos != std::string::npos) {
        if (out.quest_id.empty()) {
            std::string id_cand = out.description.substr(0, colon_pos);
            while (!id_cand.empty() && std::isspace(
                       static_cast<unsigned char>(id_cand.front()))) {
                id_cand.erase(id_cand.begin());
            }
            while (!id_cand.empty() && std::isspace(
                       static_cast<unsigned char>(id_cand.back()))) {
                id_cand.pop_back();
            }
            out.quest_id = id_cand;
        }
        after_colon = out.description.substr(colon_pos + 1);
    } else {
        after_colon = out.description;
    }

    while (!after_colon.empty() && std::isspace(
               static_cast<unsigned char>(after_colon.front()))) {
        after_colon.erase(after_colon.begin());
    }
    while (!after_colon.empty() && std::isspace(
               static_cast<unsigned char>(after_colon.back()))) {
        after_colon.pop_back();
    }
    out.objective = after_colon;

    std::string lower_after = after_colon;
    for (char& c : lower_after) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    size_t bring_pos = lower_after.find("bring ");
    size_t to_pos = lower_after.find(" to ");
    if (bring_pos != std::string::npos && to_pos != std::string::npos &&
        to_pos > bring_pos + 6) {
        std::string item_part = after_colon.substr(
            bring_pos + 6, to_pos - (bring_pos + 6));
        std::string npc_part = after_colon.substr(to_pos + 4);

        while (!item_part.empty() && std::isspace(
                   static_cast<unsigned char>(item_part.front()))) {
            item_part.erase(item_part.begin());
        }
        while (!item_part.empty() && std::isspace(
                   static_cast<unsigned char>(item_part.back()))) {
            item_part.pop_back();
        }

        while (!npc_part.empty() && (std::isspace(
                   static_cast<unsigned char>(npc_part.back())) ||
                   npc_part.back() == '.')) {
            npc_part.pop_back();
        }
        while (!npc_part.empty() && std::isspace(
                   static_cast<unsigned char>(npc_part.front()))) {
            npc_part.erase(npc_part.begin());
        }

        out.target_item = item_part;
        out.target_npc = npc_part;

        int count = 0;
        for (size_t i = 0; i < item_part.size(); ++i) {
            if (std::isdigit(static_cast<unsigned char>(item_part[i]))) {
                size_t start = i;
                while (i < item_part.size() &&
                       std::isdigit(
                           static_cast<unsigned char>(item_part[i]))) {
                    i++;
                }
                try {
                    count = std::stoi(item_part.substr(start, i - start));
                } catch (...) {
                    count = 1;
                }
                break;
            }
        }
        out.required_items = (count > 0) ? count : 1;
    } else {
        int count = 0;
        for (size_t i = 0; i < after_colon.size(); ++i) {
            if (std::isdigit(static_cast<unsigned char>(after_colon[i]))) {
                size_t start = i;
                while (i < after_colon.size() &&
                       std::isdigit(
                           static_cast<unsigned char>(after_colon[i]))) {
                    i++;
                }
                try {
                    count = std::stoi(after_colon.substr(start, i - start));
                } catch (...) {
                    count = 0;
                }
                break;
            }
        }
        out.required_items = count;
        out.target_item = after_colon;
    }

    if (out.progress.empty() && out.required_items > 0) {
        out.progress = std::to_string(out.required_items) + " required";
    }
}

bool parseQuestResponse(const std::string& raw_response, QuestInfo& out) {
    const std::string prefix = "OK ";
    if (raw_response.substr(0, prefix.size()) != prefix) {
        return false;
    }

    std::string json_str = raw_response.substr(prefix.size());

    try {
        json j = json::parse(json_str);

        if (!j.is_object()) return false;

        if (j.contains("quest_id") && j["quest_id"].is_string()) {
            out.quest_id = j["quest_id"].get<std::string>();
        } else if (j.contains("id") && j["id"].is_string()) {
            out.quest_id = j["id"].get<std::string>();
        } else if (j.contains("id") && j["id"].is_number()) {
            out.quest_id = std::to_string(j["id"].get<int>());
        }

        if (j.contains("description") && j["description"].is_string())
            out.description = j["description"].get<std::string>();
        if (j.contains("reward") && j["reward"].is_string())
            out.reward = j["reward"].get<std::string>();
        if (j.contains("status") && j["status"].is_string())
            out.status = j["status"].get<std::string>();
        if (j.contains("progress") && j["progress"].is_string())
            out.progress = j["progress"].get<std::string>();

        parseQuestObjective(out);

        return true;
    } catch (const json::exception& e) {
        std::cerr << "JSON parse error (quest): " << e.what() << std::endl;
        return false;
    }
}

bool parseQuestsResponse(const std::string& raw_response,
                         std::vector<QuestInfo>& out) {
    const std::string prefix = "OK ";
    if (raw_response.substr(0, prefix.size()) != prefix) {
        return false;
    }

    std::string json_str = raw_response.substr(prefix.size());

    try {
        json j = json::parse(json_str);

        if (!j.is_array()) return false;

        for (const auto& item : j) {
            QuestInfo q;
            if (item.contains("quest_id") && item["quest_id"].is_string()) {
                q.quest_id = item["quest_id"].get<std::string>();
            } else if (item.contains("id") && item["id"].is_string()) {
                q.quest_id = item["id"].get<std::string>();
            } else if (item.contains("id") && item["id"].is_number()) {
                q.quest_id = std::to_string(item["id"].get<int>());
            }

            if (item.contains("description") && item["description"].is_string())
                q.description = item["description"].get<std::string>();
            if (item.contains("reward") && item["reward"].is_string())
                q.reward = item["reward"].get<std::string>();
            if (item.contains("status") && item["status"].is_string())
                q.status = item["status"].get<std::string>();
            if (item.contains("progress") && item["progress"].is_string())
                q.progress = item["progress"].get<std::string>();

            parseQuestObjective(q);
            out.push_back(q);
        }

        return true;
    } catch (const json::exception& e) {
        std::cerr << "JSON parse error (quests): " << e.what() << std::endl;
        return false;
    }
}

bool parseInventoryResponse(const std::string& raw_response,
                            std::vector<std::string>& out) {
    const std::string prefix = "OK ";
    if (raw_response.substr(0, prefix.size()) != prefix) {
        return false;
    }

    std::string json_str = raw_response.substr(prefix.size());

    try {
        json j = json::parse(json_str);

        if (!j.is_array()) return false;

        for (const auto& item : j) {
            if (item.is_string()) {
                out.push_back(item.get<std::string>());
            }
        }

        return true;
    } catch (const json::exception& e) {
        std::cerr << "JSON parse error (inventory): " << e.what() << std::endl;
        return false;
    }
}

bool parseUseResponse(const std::string& raw_response, UseInfo& out) {
    const std::string prefix = "OK ";
    if (raw_response.substr(0, prefix.size()) != prefix) {
        return false;
    }

    std::string json_str = raw_response.substr(prefix.size());

    try {
        json j = json::parse(json_str);

        if (!j.is_object()) return false;

        if (j.contains("used") && j["used"].is_string()) {
            out.used = j["used"].get<std::string>();
        } else {
            return false;
        }

        if (j.contains("hp") && j["hp"].is_number())
            out.hp = j["hp"].get<int>();
        if (j.contains("max_hp") && j["max_hp"].is_number())
            out.max_hp = j["max_hp"].get<int>();
        if (j.contains("attack") && j["attack"].is_number())
            out.attack = j["attack"].get<int>();
        if (j.contains("received") && j["received"].is_string())
            out.received = j["received"].get<std::string>();

        return true;
    } catch (const json::exception&) {
        return false;
    }
}

bool parseCombatResponse(const std::string& raw_response, CombatInfo& out) {
    std::string json_str;
    if (raw_response.rfind("OK ", 0) == 0) {
        json_str = raw_response.substr(3);
    } else if (raw_response.rfind("EVT ROOM COMBAT ", 0) == 0) {
        json_str = raw_response.substr(16);
    } else {
        return false;
    }

    try {
        json j = json::parse(json_str);
        if (!j.is_object()) return false;

        if (j.contains("attacker") && j["attacker"].is_string())
            out.attacker = j["attacker"].get<std::string>();
        if (j.contains("target") && j["target"].is_string())
            out.target = j["target"].get<std::string>();
        if (j.contains("attacker_hp") && j["attacker_hp"].is_number())
            out.attacker_hp = j["attacker_hp"].get<int>();
        if (j.contains("target_hp") && j["target_hp"].is_number())
            out.target_hp = j["target_hp"].get<int>();
        if (j.contains("damage") && j["damage"].is_number())
            out.damage = j["damage"].get<int>();
        if (j.contains("damage_taken") && j["damage_taken"].is_number())
            out.damage_taken = j["damage_taken"].get<int>();
        if (j.contains("status") && j["status"].is_string())
            out.status = j["status"].get<std::string>();
        if (j.contains("room") && j["room"].is_string())
            out.room = j["room"].get<std::string>();

        return true;
    } catch (const json::exception&) {
        return false;
    }
}

static void formatJsonValue(const json& j, const std::string& prefix,
                            std::vector<std::string>& lines) {
    if (j.is_object()) {
        for (auto& [key, val] : j.items()) {
            std::string label = key;
            if (!label.empty())
                label[0] = static_cast<char>(std::toupper(label[0]));
            if (val.is_object() || val.is_array()) {
                formatJsonValue(val, prefix + label + ".", lines);
            } else if (val.is_string()) {
                lines.push_back(prefix + label + ": " +
                                val.get<std::string>());
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
    const std::string prefix = "OK ";
    if (raw_response.substr(0, prefix.size()) != prefix) {
        return false;
    }

    std::string json_str = raw_response.substr(prefix.size());

    try {
        json j = json::parse(json_str);
        formatJsonValue(j, "", out_lines);
        return true;
    } catch (const json::exception&) {
        return false;
    }
}

std::vector<std::string> formatQuestInfo(const QuestInfo& info) {
    std::vector<std::string> lines;
    if (!info.quest_id.empty())
        lines.push_back("Quest: " + info.quest_id);
    if (!info.description.empty())
        lines.push_back("Description: " + info.description);
    if (!info.objective.empty() && info.objective != info.description)
        lines.push_back("Objective: " + info.objective);
    if (info.required_items > 0) {
        lines.push_back("Required items: " +
                        std::to_string(info.required_items));
    }
    if (!info.target_item.empty())
        lines.push_back("Target item: " + info.target_item);
    if (!info.target_npc.empty())
        lines.push_back("Deliver to: " + info.target_npc);
    if (!info.progress.empty())
        lines.push_back("Progress: " + info.progress);
    if (!info.reward.empty())
        lines.push_back("Reward: " + info.reward);
    if (!info.status.empty())
        lines.push_back("Status: " + info.status);
    return lines;
}

std::vector<std::string> formatUseInfo(const UseInfo& info) {
    std::vector<std::string> lines;
    std::string item_name = info.used;
    if (!item_name.empty()) {
        item_name[0] = static_cast<char>(std::toupper(item_name[0]));
    }
    lines.push_back("Used: " + item_name);

    if (info.max_hp > 0) {
        lines.push_back("HP: " + std::to_string(info.hp) + "/" +
                        std::to_string(info.max_hp));
    } else if (info.hp > 0) {
        lines.push_back("HP: " + std::to_string(info.hp));
    }

    if (info.attack > 0) {
        lines.push_back("Attack: " + std::to_string(info.attack));
    }

    if (!info.received.empty()) {
        std::string rec = info.received;
        if (!rec.empty()) {
            rec[0] = static_cast<char>(std::toupper(rec[0]));
        }
        lines.push_back("Received: " + rec);
    }

    return lines;
}

std::vector<std::string> formatCombatInfo(const CombatInfo& info) {
    std::vector<std::string> lines;
    std::string target_name = info.target;
    if (!target_name.empty()) {
        target_name[0] = static_cast<char>(std::toupper(target_name[0]));
    }

    lines.push_back("Combat vs " + target_name + ":");
    lines.push_back("  Damage dealt: " + std::to_string(info.damage) +
                    " (" + target_name + " HP: " +
                    std::to_string(info.target_hp) + ")");

    if (info.status == "victory") {
        lines.push_back("  Status: Victory! " + target_name + " defeated.");
    } else {
        lines.push_back("  Damage taken: " + std::to_string(info.damage_taken) +
                        " (Your HP: " + std::to_string(info.attacker_hp) + ")");
        if (info.status == "defeated") {
            std::string respawn =
                info.room.empty() ? "respawn point" : info.room;
            lines.push_back("  Status: Defeated! You respawned in " +
                            respawn + ".");
        } else {
            lines.push_back("  Status: In combat");
        }
    }

    return lines;
}

ParsedChat parseChatCommand(const std::string& input) {
    ParsedChat result;
    size_t i = 0;
    while (i < input.size() &&
           std::isspace(static_cast<unsigned char>(input[i]))) {
        i++;
    }
    if (i >= input.size()) return result;

    size_t cmd_start = i;
    while (i < input.size() &&
           !std::isspace(static_cast<unsigned char>(input[i]))) {
        i++;
    }
    std::string cmd = input.substr(cmd_start, i - cmd_start);
    for (char& c : cmd) c = static_cast<char>(std::toupper(c));
    if (cmd != "CHAT") return result;

    while (i < input.size() &&
           std::isspace(static_cast<unsigned char>(input[i]))) {
        i++;
    }
    if (i >= input.size()) return result;

    size_t scope_start = i;
    while (i < input.size() &&
           !std::isspace(static_cast<unsigned char>(input[i]))) {
        i++;
    }
    std::string scope = input.substr(scope_start, i - scope_start);
    for (char& c : scope) c = static_cast<char>(std::toupper(c));

    std::string channel;
    std::string wire_scope;
    if (scope == "ROOM") {
        channel = "ROOM";
        wire_scope = "ROOM";
    } else if (scope == "GLOBAL") {
        channel = "GLOBAL";
        wire_scope = "GLOBAL";
    } else if (scope == "PARTY" || scope == "GROUP") {
        channel = "PARTY";
        wire_scope = "GROUP";
    } else {
        return result;
    }

    while (i < input.size() &&
           std::isspace(static_cast<unsigned char>(input[i]))) {
        i++;
    }
    if (i >= input.size()) return result;

    std::string msg = input.substr(i);
    while (!msg.empty() &&
           std::isspace(static_cast<unsigned char>(msg.back()))) {
        msg.pop_back();
    }
    if (msg.empty()) return result;

    result.is_chat = true;
    result.channel = channel;
    result.message = msg;
    result.command_to_send = "CHAT " + wire_scope + " " + msg;
    return result;
}
