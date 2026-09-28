#pragma once

#include <string>

#include <ftxui/dom/elements.hpp>

// Charge une image (PNG, JPG, BMP, GIF, TGA, PSD, HDR, PNM) et renvoie un
// Element FTXUI qui la dessine dans le terminal (demi-blocs "▀" en couleurs
// vraies : chaque cellule affiche 2 pixels l'un au-dessus de l'autre).
//
//   largeur_max / hauteur_max : taille maximale en CELLULES du terminal
//   (0 = pas de limite sur cet axe). L'image est réduite en gardant ses
//   proportions, jamais agrandie.
//
// Utilisation :
//   vbox({ text("Alice :"), put_image("photo.png") })
//   put_image("photo.png", 40, 15) | center
//
// Le fichier n'est lu qu'une seule fois (mis en cache par chemin), on peut donc
// appeler put_image() à chaque rendu. À appeler depuis le thread de l'interface.
// Si le fichier est illisible, un texte d'erreur rouge est renvoyé à la place.
ftxui::Element put_image(const std::string& chemin, int largeur_max = 0, int hauteur_max = 0, bool garder_ratio = false);
