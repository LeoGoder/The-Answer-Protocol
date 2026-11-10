#include "map_parse.hpp"
#include "helper.hpp"
#include <string>
#include <cstdio>
#include <vector>

static std::string get_map_string(std::string path) {
    FILE *raw_data = fopen(GetAssetPath(path).c_str(), "r");

    if (!raw_data) {
        printf("ERROR while opening the file\n");
            return "";
    }
    fseek(raw_data, 0, SEEK_END);
    long size = ftell(raw_data);

    rewind(raw_data);
    std::string string_data(size, '\0');

    fread(&string_data[0], sizeof(char), size, raw_data);
    fclose(raw_data);

    return string_data;
}

void parse_map(MapData &data) {
    std::string hub = get_map_string("hub.json");
    std::string fejka = get_map_string("fejka.json");
    std::string fugoumitak = get_map_string("fugoumitak.json");
    std::string jeetomal = get_map_string("jeetomal.json");
    std::string komolexos = get_map_string("komolexos.json");
    std::string kuggis = get_map_string("kuggis.json");
    std::string smycka = get_map_string("smycka.json");
    std::string vinterfint = get_map_string("vinterfint.json");

    if (!hub.empty())
        data.hub = extract_layers(hub);
    if (!fejka.empty())
        data.fejka = extract_layers(fejka);
    if (!fugoumitak.empty())
        data.fugoumitak = extract_layers(fugoumitak);
    if (!jeetomal.empty())
        data.jeetomal = extract_layers(jeetomal);
    if (!komolexos.empty())
        data.komolexos = extract_layers(komolexos);
    if (!kuggis.empty())
        data.kuggis = extract_layers(kuggis);
    if (!smycka.empty())
        data.smycka = extract_layers(smycka);
    if (!vinterfint.empty())
        data.vinterfint = extract_layers(vinterfint);
}
