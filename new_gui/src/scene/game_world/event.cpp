#include <cstddef>
#include <memory>
#include <vector>
#include <cstdlib>
#include <string>
#include <utility>
#include "game_world.hpp"
#include "player.hpp"
#include "helper.hpp"

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

void GameWorld::update_server_players(std::string msg) {
    size_t pos = msg.rfind("players=");

    if (pos == std::string::npos)
        return;
    this->server_players = std::atoi(msg.c_str() + pos + 8);
}


static void remove_id(std::vector<std::string> &list, const std::string &id) {
    for (size_t i = 0; i < list.size(); i++) {
        if (list[i] == id) {
            list.erase(list.begin() + i);
            return;
        }
    }
}

static std::string word_after(const std::string &msg, const std::string &prefix, size_t skip) {
    size_t pos = msg.find(prefix);

    if (pos == std::string::npos)
        return "";
    std::string rest = msg.substr(pos + prefix.length());
    std::vector<std::string> words = split(rest, ' ');

    if (words.size() <= skip)
        return "";
    return words[skip];
}

void GameWorld::update_items(std::string msg) {
    std::vector<std::string> inventory = this->player.get_inventory();
    std::string id;

    id = word_after(msg, "OK taken=", 0);
    if (id != "") {
        remove_id(this->room.items, id);
        inventory.push_back(id);
    }
    id = word_after(msg, "OK dropped=", 0);
    if (id != "") {
        remove_id(inventory, id);
        this->room.items.push_back(id);
    }
    id = word_after(msg, "EVT ROOM ITEM TAKE ", 1);
    if (id != "")
        remove_id(this->room.items, id);
    id = word_after(msg, "EVT ROOM ITEM DROP ", 1);
    if (id != "")
        this->room.items.push_back(id);
    id = word_after(msg, "EVT ROOM ITEM GONE ", 0);
    if (id != "")
        remove_id(this->room.items, id);
    id = word_after(msg, "EVT ROOM ITEM SPAWN ", 0);
    if (id != "")
        this->room.items.push_back(id);
    this->player.set_inventory(inventory);
}