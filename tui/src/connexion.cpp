#include "connexion.hpp"

#include <string>

// En-têtes de la bibliothèque FTXUI
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

bool ecranConnexion(InfosConnexion& infos) {
    // 1. Création des composants interactifs (les champs de texte)
    //    Ils écrivent directement dans la structure "infos"
    Component input_pseudo = Input(&infos.pseudo, "Ex: Alice");
    Component input_ip     = Input(&infos.ip, "Ex: 127.0.0.1");
    Component input_port   = Input(&infos.port, "Ex: 4242");

    // 2. Écran + état
    auto screen = ScreenInteractive::TerminalOutput();
    bool valide = false;
    std::string erreur;

    // 3. Bouton : on ne valide que si le pseudo n'est pas vide
    auto on_connect = [&] {
        if (infos.pseudo.empty()) {
            erreur = "Le pseudo ne peut pas etre vide.";
            return;
        }
        valide = true;
        screen.Exit(); // Quitte la boucle d'affichage
    };
    Component btn_connect = Button("Se connecter", on_connect, ButtonOption::Animated());

    // 4. Conteneur vertical (navigation avec les flèches / Tab)
    Component container = Container::Vertical({
        input_pseudo,
        input_ip,
        input_port,
        btn_connect,
    });

    // 5. Rendu visuel
    Component renderer = Renderer(container, [&] {
        return vbox({
            text(" THE ANSWER PROTOCOL - TUI ") | bold | center,
            separator(),
            hbox({text(" Pseudo : "), input_pseudo->Render()}),
            hbox({text(" IP     : "), input_ip->Render()}),
            hbox({text(" Port   : "), input_port->Render()}),
            separator(),
            erreur.empty() ? emptyElement()
                           : text(" " + erreur) | color(Color::Red),
            btn_connect->Render() | center,
        }) | border | flex_shrink;
    });

    // 6. Le programme bloque ici tant que l'utilisateur n'a pas cliqué (ou fait Ctrl+C)
    screen.Loop(renderer);

    return valide;
}
