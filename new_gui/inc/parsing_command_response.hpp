#pragma once
#include <vector>
#include <string>
#include "game_world.hpp"

void parse_look_response(std::vector<std::string> &response, Room &room, Texture2D *font, Texture2D *sprite);
void parse_move_response(std::vector<std::string> &response, Room &room, NetCommand &command, int fd);
