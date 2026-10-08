#include "../includes/parssing.hpp"

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

bool parseQuestResponse(const std::string& raw_response, QuestInfo& out) {
    const std::string prefix = "OK ";
    if (raw_response.substr(0, prefix.size()) != prefix) {
        return false;
    }

    std::string json_str = raw_response.substr(prefix.size());

    try {
        json j = json::parse(json_str);

        if (!j.is_object()) return false;
        if (!j.contains("quest_id")) return false;

        if (j.contains("quest_id") && j["quest_id"].is_string())
            out.quest_id = j["quest_id"].get<std::string>();
        if (j.contains("description") && j["description"].is_string())
            out.description = j["description"].get<std::string>();
        if (j.contains("reward") && j["reward"].is_string())
            out.reward = j["reward"].get<std::string>();
        if (j.contains("status") && j["status"].is_string())
            out.status = j["status"].get<std::string>();
        if (j.contains("progress") && j["progress"].is_string())
            out.progress = j["progress"].get<std::string>();

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
            if (item.contains("quest_id") && item["quest_id"].is_string())
                q.quest_id = item["quest_id"].get<std::string>();
            if (item.contains("description") && item["description"].is_string())
                q.description = item["description"].get<std::string>();
            if (item.contains("reward") && item["reward"].is_string())
                q.reward = item["reward"].get<std::string>();
            if (item.contains("status") && item["status"].is_string())
                q.status = item["status"].get<std::string>();
            if (item.contains("progress") && item["progress"].is_string())
                q.progress = item["progress"].get<std::string>();
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
    lines.push_back("Quest: " + info.quest_id);
    lines.push_back("Description: " + info.description);
    if (!info.progress.empty())
        lines.push_back("Progress: " + info.progress);
    if (!info.reward.empty())
        lines.push_back("Reward: " + info.reward);
    if (!info.status.empty())
        lines.push_back("Status: " + info.status);
    return lines;
}
