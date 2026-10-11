#pragma once
#include "save_load.hpp"

void put_letter_in_player_name(int save_slot_choice, GameData &data, char c);
void delete_last_character(int save_slot_choice, GameData &data);
void clear_all_character(int save_slot_choice, GameData &data);
