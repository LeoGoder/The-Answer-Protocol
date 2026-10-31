#include "game_world.hpp"
#include "map.hpp"
#include "map_parse.hpp"
#include "helper.hpp"
#include "raylib.h"
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <vector>

void GameWorld::draw_island(std::vector<std::vector<int>> &island) {
    int index = 0;
    Vector2 tile_size = {TILE_WIDTH, TILE_HEIGHT};

    for (size_t i = 0; i < island.size() - 2; i++) {
        for (int y = 0; y < MAP_HEIGHT; y ++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                int tile_id = island[i][y * MAP_WIDTH + x];
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
            draw_island(this->map_data.hub);
            break;
        case komolexos:
            draw_island(this->map_data.komolexos);
            break;
        case fugoumitak:
            draw_island(this->map_data.fugoumitak);
            break;
        case fejka:
            draw_island(this->map_data.fejka);
            break;
        case jeetomal:
            draw_island(this->map_data.jeetomal);
            break;
        case kuggis:
            draw_island(this->map_data.kuggis);
            break;
        case smycka:
            draw_island(this->map_data.smycka);
            break;
        case vinterfint:
            draw_island(this->map_data.vinterfint);
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
