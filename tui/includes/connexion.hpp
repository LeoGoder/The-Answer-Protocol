#pragma once

#include <string>

struct InfosConnexion {
    std::string pseudo;
    std::string ip   = "127.0.0.1";
    std::string port = "4242";
};

bool ecranConnexion(InfosConnexion& infos,
                    const std::string& erreur_initiale = "");
