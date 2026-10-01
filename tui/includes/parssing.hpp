#pragma once

#include <map>
#include <string>
#include <vector>

// Informations d'une room renvoyées par la commande LOOK
struct RoomInfo {
    std::string id;
    std::string name;
    std::string description;
    std::map<std::string, std::string> exits;   // direction -> room_id
    std::vector<std::string> players;
    std::vector<std::string> items;
    std::vector<std::string> npcs;
};

// Parse la réponse JSON du serveur pour la commande LOOK.
// La réponse attendue est de la forme : OK {"room":{...},"players":[...],...}
// Retourne true si le parsing a réussi, false sinon.
bool parseLookResponse(const std::string& raw_response, RoomInfo& out);

// Formate un RoomInfo en lignes lisibles pour l'affichage dans le terminal.
// Retourne un vecteur de strings, une par ligne.
std::vector<std::string> formatRoomInfo(const RoomInfo& info);
