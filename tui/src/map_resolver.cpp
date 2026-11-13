// Room map asset resolution

#include "../includes/map_resolver.hpp"

#include <cctype>
#include <filesystem>  // NOLINT(build/c++17)
#include <string>
#include <vector>

/*
 * Resolves room map image path using a 4-tier fallback cascade:
 * 1. Exact sanitized full room ID (e.g. "forest_clearing.png")
 * 2. Short ID suffix after last dot (e.g. "clearing.png")
 * 3. Sanitized display name (e.g. "the_dark_cave.png")
 * 4. Default fallback image ("assets/maps/default.png")
 */
std::string resolveMapPath(const std::string& room_id,
                           const std::string& room_name) {
    std::vector<std::string> candidates;

    if (!room_id.empty()) {
        std::string raw_id = room_id;
        // Extract base identifier by stripping dot-separated namespace or "room_" prefix
        size_t dot = raw_id.find('.');
        if (dot != std::string::npos) {
            raw_id = raw_id.substr(dot + 1);
        } else if (raw_id.rfind("room_", 0) == 0) {
            raw_id = raw_id.substr(5);
        }

        // Normalize characters: lowercase and underscores
        std::string clean = raw_id;
        for (char& c : clean) {
            if (c == ' ' || c == '-' || c == '.') {
                c = '_';
            } else {
                c = static_cast<char>(std::tolower(c));
            }
        }

        if (clean.rfind("room_", 0) == 0) {
            candidates.push_back("assets/maps/" + clean + ".png");
        } else {
            candidates.push_back("assets/maps/room_" + clean + ".png");
            candidates.push_back("assets/maps/" + clean + ".png");
        }

        // Legacy alias for hub starting room
        if (clean == "hub") {
            candidates.push_back("assets/maps/room_start.png");
        }

        std::string id_clean = room_id;
        for (char& c : id_clean) {
            if (c == '.' || c == ' ' || c == '-') {
                c = '_';
            } else {
                c = static_cast<char>(std::tolower(c));
            }
        }
        if (id_clean != clean) {
            candidates.push_back("assets/maps/" + id_clean + ".png");
        }
    }

    if (!room_name.empty()) {
        std::string clean_name = room_name;
        for (char& c : clean_name) {
            if (c == ' ' || c == '-' || c == '.') {
                c = '_';
            } else {
                c = static_cast<char>(std::tolower(c));
            }
        }

        const std::string suffix = "_room";
        if (clean_name.size() > suffix.size() &&
            clean_name.substr(clean_name.size() - suffix.size()) == suffix) {
            std::string prefix =
                clean_name.substr(0, clean_name.size() - suffix.size());
            candidates.push_back("assets/maps/room_" + prefix + ".png");
        } else if (clean_name.rfind("room_", 0) == 0) {
            candidates.push_back("assets/maps/" + clean_name + ".png");
        } else {
            candidates.push_back("assets/maps/room_" + clean_name + ".png");
            candidates.push_back("assets/maps/" + clean_name + ".png");
        }
    }

    // Check if any candidate exists on disk
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }

    // Fallback default image
    if (std::filesystem::exists("assets/maps/default.png")) {
        return "assets/maps/default.png";
    }

    if (!candidates.empty()) {
        return candidates[0];
    }

    return "assets/maps/default.png";
}
