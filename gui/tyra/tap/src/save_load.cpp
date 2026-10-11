#include "save_load.hpp"
#include "debug/debug.hpp"
#include <cstddef>
#include <cstdio>
#include <libmc.h>
#include <fcntl.h>
#include <stdio.h>


int save_game(int slot_index, GameData &data) {
    int res = 0;
    int fd = 0;
    int type = 0, free = 0, format = 0;
    char dir_path[32];
    char file_path[64];

    snprintf(dir_path, sizeof(dir_path), "TAP_%02d", slot_index);
    snprintf(file_path, sizeof(file_path), "%s/save.bin", dir_path);
    mcGetInfo(0, 0, &type, &free, &format);
    mcSync(0, NULL, &res);
    if (res != -1 && res != 0) {
        TYRA_LOG("SAVE_FAIL Card unoformated or not ready, res=", res);
        return SAVE_FAIL;
    }
    mcMkDir(0, 0, dir_path);
    mcSync(0, NULL, &res);
    mcOpen(0, 0, file_path, O_WRONLY | O_CREAT);
    mcSync(0, NULL, &res);
    if (res < 0) {
        TYRA_LOG("SAVE_FAIL create save file in memory card, res=", res);
        return SAVE_FAIL;
    }
    fd = res;
    mcWrite(fd, &data, sizeof(GameData));
    mcSync(0, NULL, &res);
    mcClose(fd);
    mcSync(0, NULL, &res);
    return SAVE_SUCCESS;
}

int load_data(int slot_index, GameData &data) {
    int res = 0;
    int fd = 0;
    int type = 0, free = 0, format = 0;
    char file_path[64];

    mcGetInfo(0, 0, &type, &free, &format);
    mcSync(0, NULL, &res);
    if (res != -1 && res != 0)
        return false;
    snprintf(file_path, sizeof(file_path), "TAP_%02d/save.bin", slot_index);
    mcOpen(0, 0, file_path, O_RDONLY);
    mcSync(0, NULL, &res);
    fd  = res;
    if (res < 0) {
        mcClose(fd);
        mcSync(0, NULL, &res);
        return LOAD_FAIL;
    }
    mcRead(fd, &data, sizeof(GameData));
    mcSync(0, NULL, &res);
    if (res != sizeof(GameData)) {
        mcClose(fd);
        mcSync(0, NULL, &res);
        return LOAD_FAIL;
    }
    mcClose(fd);
    mcSync(0, NULL, &res);
    return LOAD_SUCCESS;
}

bool is_slot_used(int slot_index) {
    int res = 0;
    int fd = 0;
    int type = 0, free = 0, format = 0;
    char file_path[64];

    mcGetInfo(0, 0, &type, &free, &format);
    mcSync(0, NULL, &res);
    if (res != -1 && res != 0) {
        TYRA_LOG("Slot_used: Failed to get info, res=", res);
        return false;
    }
    TYRA_LOG("Free space available: ", free);
    snprintf(file_path, sizeof(file_path), "TAP_%02d/save.bin", slot_index);
    mcOpen(0, 0, file_path, O_RDONLY);
    mcSync(0, NULL, &res);
    fd = res;
    if (res >= 0) {
        mcClose(fd);
        mcSync(0, NULL, &res);
        return true;
    }
    return false;
}
