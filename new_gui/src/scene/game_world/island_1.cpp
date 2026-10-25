#include "game_world.hpp"
#include <cstddef>

// donner ne parametre la data de l'ile
static void draw_island() {

}

void GameWorld::draw_game() {
    player.draw_player(this->player_name);
    if (!this->room.player.empty()) {
        for (size_t i = 0; i < this->room.player.size(); i++) {
            std::string player_room_name = this->room.player[i]->name;

            if (player_room_name != this->player_name)
                room.player[i]->draw_player(player_room_name);
        }
    }
    switch (this->map) {
        case hub:
            draw_island();
            break;
        case komolexos:
            break;
        case fugoumitak:
            break;
        case fejka:
            break;
        case jeetomal:
            break;
        case kuggis:
            break;
        case smycka:
            break;
        case vinterfint:
            break;
    }
}
