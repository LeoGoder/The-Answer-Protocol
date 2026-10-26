#include "game_world.hpp"
#include "map.hpp"
#include "map_parse.hpp"
#include "helper.hpp"
#include "raylib.h"
#include <cstddef>
#include <cstdio>

void GameWorld::draw_island(MapData &island) {
    (void)island;
    int index = 0;
    Vector2 tile_size = {TILE_WIDTH, TILE_HEIGHT};

    for (size_t i = 0; i < island.hub.size(); i++) {
        for (int y = 0; y < MAP_HEIGHT; y ++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                int tile_id = island.hub[i][y * MAP_WIDTH + x];
                if (tile_id <= 0)
                    continue;
                Vector2 position;
                position.x = x * TILE_WIDTH;
                position.y = y * TILE_HEIGHT;

                if (tile_id >= 1 && tile_id < EXTERIOR_FIRSTGRID)
                    draw_sprite_sheet(this->ground, tile_id - MAP_FIRSTGRID, this->ground.width / TILE_WIDTH, position, tile_size, 1);
                if (tile_id >= EXTERIOR_FIRSTGRID && tile_id < SHEEP_WITH_SHADOW_FIRSTGRID)
                    draw_sprite_sheet(this->exterior, tile_id - EXTERIOR_FIRSTGRID, this->exterior.width / TILE_WIDTH, position, tile_size, 1);
                if (tile_id >= SHEEP_WITH_SHADOW_FIRSTGRID && tile_id < COW_WITH_SHADOW_FIRSTGRID)
                    draw_sprite_sheet(this->sheep, tile_id - SHEEP_WITH_SHADOW_FIRSTGRID, this->sheep.width / TILE_WIDTH, position, tile_size, 1);
                if (tile_id >= COW_WITH_SHADOW_FIRSTGRID && tile_id < TREES_FIRSTGRID)
                    draw_sprite_sheet(this->calf, tile_id - COW_WITH_SHADOW_FIRSTGRID, this->calf.width / TILE_WIDTH, position, tile_size, 1);
                if (tile_id >= TREES_FIRSTGRID && tile_id < ROCK_AND_FLOWERS_FIRSTGRID)
                    draw_sprite_sheet(this->trees, tile_id - TREES_FIRSTGRID, this->trees.width / TILE_WIDTH, position, tile_size, 1);
                if (tile_id >= ROCK_AND_FLOWERS_FIRSTGRID && tile_id < FOX_WITH_SHADOW_FIRSTGRID)
                    draw_sprite_sheet(this->rock_flowers, tile_id - ROCK_AND_FLOWERS_FIRSTGRID, this->rock_flowers.width / TILE_WIDTH, position, tile_size, 1);
                if (tile_id >= FOX_WITH_SHADOW_FIRSTGRID && tile_id < HARE_IDLE_WITH_SHADOW_FIRSTGRID)
                    draw_sprite_sheet(this->fox, tile_id - FOX_WITH_SHADOW_FIRSTGRID, this->fox.width / TILE_WIDTH, position, tile_size, 1);
                index++;
            }
            index++;
        }
    }
}

void GameWorld::draw_game() {
    switch (this->map) {
        case hub:
            draw_island(this->map_data);
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
    player.draw_player(this->player_name);
    if (!this->room.player.empty()) {
        for (size_t i = 0; i < this->room.player.size(); i++) {
            std::string player_room_name = this->room.player[i]->name;
            if (player_room_name != this->player_name)
                room.player[i]->draw_player(player_room_name);
        }
    }
}
