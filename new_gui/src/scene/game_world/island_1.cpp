#include "game_world.hpp"
#include <cstddef>

void GameWorld::draw_game() {
    player.draw_player(this->player_name);
    if (!this->room.player.empty()) {
        for (size_t i = 0; i < this->room.player.size(); i++) {
            std::string player_room_name = this->room.player[i]->name;

            if (player_room_name != this->player_name)
                room.player[i]->draw_player(player_room_name);
        }
    }
}
