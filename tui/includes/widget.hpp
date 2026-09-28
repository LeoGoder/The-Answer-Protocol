#pragma once

#include <string>
#include <ftxui/dom/elements.hpp>

// Crée un widget positionné aux coordonnées (x, y) avec la largeur et
// la hauteur spécifiées (en cellules de terminal).
// Si le contenu n'est pas fourni, l'image "temp_image.png" est affichée et sa
// résolution s'adapte automatiquement à l'espace intérieur du widget.
ftxui::Element creerWidget(int x, int y, int largeur, int hauteur,
                           ftxui::Element contenu = nullptr,
                           const std::string& titre = " Image ");

// Crée un widget dynamique dont la taille et la position sont proportionnelles
// à l'écran du terminal (ex: 0.35f = 35% de la taille).
// Si ratio_x < 0 : alignement par rapport au bord droit (-0.02f = 2% du bord droit).
// Si ratio_y < 0 : alignement par rapport au bord bas.
// Quand on dézoome dans le terminal, le widget garde sa taille relative et la
// résolution de l'image augmente proportionnellement au nombre de cellules.
ftxui::Element creerWidgetProportionnel(float ratio_x, float ratio_y,
                                       float ratio_largeur, float ratio_hauteur,
                                       ftxui::Element contenu = nullptr,
                                       const std::string& titre = " Image ");

// Variantes pratiques pour afficher directement un fichier image
ftxui::Element creerWidgetImage(const std::string& chemin_image,
                                int x, int y, int largeur, int hauteur,
                                const std::string& titre = " Image ");

ftxui::Element creerWidgetImageProportionnel(const std::string& chemin_image,
                                            float ratio_x, float ratio_y,
                                            float ratio_largeur, float ratio_hauteur,
                                            const std::string& titre = " Image ");
