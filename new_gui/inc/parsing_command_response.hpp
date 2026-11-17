#pragma once
#include <vector>
#include <string>
#include "game_world.hpp"
#include "map_parse.hpp"

std::vector<std::string> parse_id_list(std::string &msg, std::string key);
std::vector<std::string> parse_quests(std::string &msg);

void parse_look_response(std::vector<std::string> &response, Room &room,
                         Texture2D *font, Texture2D *sprite, Maps &map);
void parse_move_response(std::string &player_name, Room &room,
                         NetCommand &command, int fd);
