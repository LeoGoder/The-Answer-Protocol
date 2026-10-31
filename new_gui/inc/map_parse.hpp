#pragma once

#include <vector>

#define MAP_WIDTH 60
#define MAP_HEIGHT 40
#define TILE_WIDTH 32
#define TILE_HEIGHT 32

struct MapData {
    std::vector<std::vector<int>> hub;
    std::vector<std::vector<int>> fugoumitak;
    std::vector<std::vector<int>> fejka;
    std::vector<std::vector<int>> jeetomal;
    std::vector<std::vector<int>> komolexos;
    std::vector<std::vector<int>> kuggis;
    std::vector<std::vector<int>> smycka;
    std::vector<std::vector<int>> vinterfint;
};

enum Maps {
    hub = 0,
    fugoumitak = 1,
    fejka = 2,
    jeetomal = 3,
    komolexos = 4,
    kuggis = 5,
    smycka = 6,
    vinterfint = 7,
};
void parse_map(MapData &data);
