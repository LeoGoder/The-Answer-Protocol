#include "save_load.hpp"
#include "debug/debug.hpp"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libmc.h>

int save_game(int slot_index, GameData &data) {
    int res = 0;
    int type = 0, free = 0, format = 0;
    char dir_path[32];
    char file_path[64];

    snprintf(dir_path, sizeof(dir_path), "mc0:TAP_%02d", slot_index);
    snprintf(file_path, sizeof(file_path), "%s/save.bin", dir_path);

    mcGetInfo(0, 0, &type, &free, &format);
    mcSync(0, NULL, &res);
    if (res != -1 && res != 0) {
        TYRA_LOG("SAVE_FAIL Card unformatted or not ready, res=", res);
        return SAVE_FAIL;
    }

    mkdir(dir_path, 0777);

    int fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) {
        TYRA_LOG("SAVE_FAIL create save file in memory card, fd=", fd);
        return SAVE_FAIL;
    }

    data.player_name[P_NAME_LEN - 1] = '\0';
    int written = write(fd, &data, sizeof(GameData));
    close(fd);

    TYRA_LOG("SAVE: write, bytes=", written);
    if (written != sizeof(GameData)) {
        TYRA_LOG("SAVE_FAIL can't write save file in memory card, written=", written);
        return SAVE_FAIL;
    }

    return SAVE_SUCCESS;
}

int load_data(int slot_index, GameData &data) {
    int res = 0;
    int type = 0, free = 0, format = 0;
    char file_path[64];

    memset(&data, 0, sizeof(GameData));

    mcGetInfo(0, 0, &type, &free, &format);
    mcSync(0, NULL, &res);
    if (res != -1 && res != 0) {
        TYRA_LOG("LOAD_FAIL Card unformatted or not ready, res=", res);
        return LOAD_FAIL;
    }

    snprintf(file_path, sizeof(file_path), "mc0:TAP_%02d/save.bin", slot_index);
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        TYRA_LOG("LOAD_FAIL failed to open save file in memory card, fd=", fd);
        return LOAD_FAIL;
    }

    int read_bytes = read(fd, &data, sizeof(GameData));
    close(fd);

    TYRA_LOG("LOAD: read, bytes=", read_bytes);
    if (read_bytes <= 0) {
        TYRA_LOG("LOAD_FAIL failed to read save file in memory card, bytes=", read_bytes);
        return LOAD_FAIL;
    }

    data.player_name[P_NAME_LEN - 1] = '\0';
    return LOAD_SUCCESS;
}

bool is_slot_used(int slot_index) {
    int res = 0;
    int type = 0, free = 0, format = 0;
    char file_path[64];

    mcGetInfo(0, 0, &type, &free, &format);
    mcSync(0, NULL, &res);
    if (res != -1 && res != 0) {
        TYRA_LOG("Slot_used: Failed to get info, res=", res);
        return false;
    }
    TYRA_LOG("Free space available: ", free);

    snprintf(file_path, sizeof(file_path), "mc0:TAP_%02d/save.bin", slot_index);
    int fd = open(file_path, O_RDONLY);
    if (fd >= 0) {
        close(fd);
        return true;
    }
    return false;
}
