/**
 * @file map_resolver.hpp
 * @brief Room map asset resolution strategies and fallback mechanisms.
 */

#pragma once

#include <string>

/**
 * @brief Resolves the disk path to a room map image using a multi-tiered fallback cascade.
 *
 * Checks candidates in the following priority:
 * 1. Exact sanitized full room ID (e.g., "forest_clearing.png")
 * 2. Short ID suffix after last dot (e.g., "clearing.png" or "room_clearing.png")
 * 3. Sanitized room display name (e.g., "the_dark_cave.png")
 * 4. Default fallback asset ("assets/maps/default.png")
 *
 * @param room_id Canonical room identifier received from LOOK or MOVE response.
 * @param room_name Optional human-readable room name.
 * @return Resolved filesystem path to an existing PNG map image asset.
 */
std::string resolveMapPath(const std::string& room_id,
                           const std::string& room_name = "");

// Convenience alias forwarding directly to resolveMapPath
inline std::string roomToMapPath(const std::string& room_id,
                                 const std::string& room_name = "") {
    return resolveMapPath(room_id, room_name);
}
