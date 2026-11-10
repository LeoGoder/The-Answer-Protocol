#include <cstddef>
#include <memory>
#include <vector>
#include <utility>
#include "game_world.hpp"
#include "player.hpp"

void GameWorld::room_presence_enter(std::string new_player) {
    this->room.player.push_back(std::make_unique<Player>(&this->font, new_player, &this->player_sprite));
}

static int find_player_to_delete(const std::string &player_to_find, std::vector<std::unique_ptr<Player>> &players) {
    int res = -1;

    for (std::size_t i = 0; i < players.size(); i++) {
        if (players[i]->name == player_to_find) {
            res = i;
            break;
        }
    }
    return res;
}

void GameWorld::room_presence_leave(std::string delete_player) {
    int index = find_player_to_delete(delete_player, this->room.player);

    if (index != -1 && !this->room.player.empty()) {
        std::swap(this->room.player[index], this->room.player.back());
        this->room.player.pop_back();
    }
}
