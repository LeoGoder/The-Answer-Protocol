#pragma  once
#include <libmc.h>
#include <fcntl.h>
#define LOAD_SUCCESS 1
#define LOAD_FAIL 2
#define SAVE_SUCCESS 3
#define SAVE_FAIL 4

typedef struct GameData {
    char player_name[32];
} GameData;

int save_game(int slot_index, GameData &data);
int load_data(int slot_index, GameData &data);
bool is_slot_used(int slot_index);
