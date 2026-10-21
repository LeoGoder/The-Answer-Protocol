#include "../includes/map_resolver.hpp"

#include <cctype>
#include <filesystem>  // NOLINT(build/c++17)
#include <string>
#include <vector>

std::string resolveMapPath(const std::string& room_id,
                           const std::string& room_name) {
    std::vector<std::string> candidates;

    if (!room_name.empty()) {
        std::string clean = room_name;
        for (char& c : clean) {
            if (c == ' ' || c == '-' || c == '.') {
                c = '_';
            } else {
                c = static_cast<char>(std::tolower(c));
            }
        }

        const std::string suffix = "_room";
        if (clean.size() > suffix.size() &&
            clean.substr(clean.size() - suffix.size()) == suffix) {
            std::string prefix = clean.substr(0, clean.size() - suffix.size());
            candidates.push_back("assets/maps/room_" + prefix + ".png");
        } else if (clean.rfind("room_", 0) == 0) {
            candidates.push_back("assets/maps/" + clean + ".png");
        } else {
            candidates.push_back("assets/maps/room_" + clean + ".png");
            candidates.push_back("assets/maps/" + clean + ".png");
        }
    }

    if (!room_id.empty()) {
        std::string id_clean = room_id;
        for (char& c : id_clean) {
            if (c == '.' || c == ' ' || c == '-') {
                c = '_';
            } else {
                c = static_cast<char>(std::tolower(c));
            }
        }
        candidates.push_back("assets/maps/" + id_clean + ".png");
    }

    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }

    if (!candidates.empty()) {
        return candidates[0];
    }

    return "assets/maps/default.png";
}
