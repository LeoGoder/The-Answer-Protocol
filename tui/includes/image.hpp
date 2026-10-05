#pragma once

#include <string>

#include <ftxui/dom/elements.hpp>

ftxui::Element put_image(
    const std::string& chemin, int largeur_max = 0,
    int hauteur_max = 0, bool garder_ratio = false);
