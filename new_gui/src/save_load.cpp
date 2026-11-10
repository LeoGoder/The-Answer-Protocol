#include "save_load.hpp"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int save_game(int slot_index, GameData &data) {
    mkdir("saves", 0755);
    char dir_path[64];
    char file_path[128];

    snprintf(dir_path, sizeof(dir_path), "saves/TAP_%02d", slot_index);
    mkdir(dir_path, 0755);
    snprintf(file_path, sizeof(file_path), "%s/save.bin", dir_path);

    int fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return SAVE_FAIL;

    data.player_name[P_NAME_LEN - 1] = '\0';
    ssize_t written = write(fd, &data, sizeof(GameData));
    close(fd);

    if (written != static_cast<ssize_t>(sizeof(GameData))) return SAVE_FAIL;
    return SAVE_SUCCESS;
}

int load_data(int slot_index, GameData &data) {
    char file_path[128];
    memset(&data, 0, sizeof(GameData));

    snprintf(file_path, sizeof(file_path), "saves/TAP_%02d/save.bin", slot_index);
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) return LOAD_FAIL;

    ssize_t read_bytes = read(fd, &data, sizeof(GameData));
    close(fd);

    if (read_bytes <= 0) return LOAD_FAIL;
    data.player_name[P_NAME_LEN - 1] = '\0';
    return LOAD_SUCCESS;
}

bool is_slot_used(int slot_index) {
    char file_path[128];
    snprintf(file_path, sizeof(file_path), "saves/TAP_%02d/save.bin", slot_index);
    int fd = open(file_path, O_RDONLY);
    if (fd >= 0) {
        close(fd);
        return true;
    }
    return false;
}
