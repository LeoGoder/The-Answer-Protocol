#pragma once

#include <string>

std::string resolveMapPath(const std::string& room_id,
                           const std::string& room_name);

inline std::string roomToMapPath(const std::string& room_id,
                                 const std::string& room_name) {
    return resolveMapPath(room_id, room_name);
}
