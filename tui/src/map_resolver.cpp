// Room map asset resolution

#include "../includes/map_resolver.hpp"

#include <cctype>
#include <filesystem>  // NOLINT(build/c++17)
#include <string>
#include <system_error>
#include <vector>

// Resolves the asset directory from binary location or fallback to "."
static std::string getAssetBaseDir() {
    std::error_code ec;
    std::filesystem::path exe_path =
        std::filesystem::read_symlink("/proc/self/exe", ec);
    if (!ec) {
        std::filesystem::path candidate =
            exe_path.parent_path() / "assets" / "maps";
        if (std::filesystem::exists(candidate, ec) && !ec) {
            return exe_path.parent_path().string();
        }
    }
    return ".";
}

/*
 * Resolves room map image path using a 4-tier fallback cascade:
 * 1. Exact sanitized full room ID (e.g. "forest_clearing.png")
 * 2. Short ID suffix after last dot (e.g. "clearing.png")
 * 3. Sanitized display name (e.g. "the_dark_cave.png")
 * 4. Default fallback image ("assets/maps/default.png")
 */
std::string resolveMapPath(const std::string& room_id,
                           const std::string& room_name) {
    std::string base = getAssetBaseDir();
    std::string prefix =
        (base == ".") ? "assets/maps/" : (base + "/assets/maps/");
    std::vector<std::string> candidates;

    if (!room_id.empty()) {
        std::string raw_id = room_id;
        // Extract base ID by stripping namespace or "room_" prefix
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
                c = static_cast<char>(
                    std::tolower(static_cast<unsigned char>(c)));
            }
        }

        if (clean.rfind("room_", 0) == 0) {
            candidates.push_back(prefix + clean + ".png");
        } else {
            candidates.push_back(prefix + "room_" + clean + ".png");
            candidates.push_back(prefix + clean + ".png");
        }

        // Legacy alias for hub starting room
        if (clean == "hub") {
            candidates.push_back(prefix + "room_start.png");
        }

        std::string id_clean = room_id;
        for (char& c : id_clean) {
            if (c == '.' || c == ' ' || c == '-') {
                c = '_';
            } else {
                c = static_cast<char>(
                    std::tolower(static_cast<unsigned char>(c)));
            }
        }
        if (id_clean != clean) {
            candidates.push_back(prefix + id_clean + ".png");
        }
    }

    if (!room_name.empty()) {
        std::string clean_name = room_name;
        for (char& c : clean_name) {
            if (c == ' ' || c == '-' || c == '.') {
                c = '_';
            } else {
                c = static_cast<char>(
                    std::tolower(static_cast<unsigned char>(c)));
            }
        }

        const std::string suffix = "_room";
        if (clean_name.size() > suffix.size() &&
            clean_name.substr(clean_name.size() - suffix.size()) == suffix) {
            std::string room_pfx =
                clean_name.substr(0, clean_name.size() - suffix.size());
            candidates.push_back(prefix + "room_" + room_pfx + ".png");
        } else if (clean_name.rfind("room_", 0) == 0) {
            candidates.push_back(prefix + clean_name + ".png");
        } else {
            candidates.push_back(prefix + "room_" + clean_name + ".png");
            candidates.push_back(prefix + clean_name + ".png");
        }
    }

    // Check if any candidate exists on disk
    std::error_code ec;
    for (const auto& path : candidates) {
        ec.clear();
        if (std::filesystem::exists(path, ec) && !ec) {
            return path;
        }
    }

    // Fallback default image
    ec.clear();
    std::string default_path = prefix + "default.png";
    if (std::filesystem::exists(default_path, ec) && !ec) {
        return default_path;
    }

    if (!candidates.empty()) {
        return candidates[0];
    }

    return default_path;
}
