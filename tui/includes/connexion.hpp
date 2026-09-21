#pragma once

#include <string>

// Informations saisies par l'utilisateur sur l'écran de connexion
struct InfosConnexion {
    std::string pseudo;
    std::string ip   = "127.0.0.1";
    std::string port = "4242";
};

// Affiche l'écran de connexion (TUI).
// Retourne true si l'utilisateur a validé, false s'il a quitté (Ctrl+C).
bool ecranConnexion(InfosConnexion& infos);
