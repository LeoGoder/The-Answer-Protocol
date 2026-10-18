#pragma once
#include <tyra>
#include "save_load.hpp"
#include "save_load.hpp"

void put_letter_in_player_name(GameData &data, char c);
void delete_last_character(GameData &data);
void clear_all_character(GameData &data);
void keyboard_input(Tyra::Pad &pad, int save_slot_choice, GameData &data);
