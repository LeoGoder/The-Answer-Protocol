#include "../includes/connexion.hpp"

#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using ftxui::bold;
using ftxui::border;
using ftxui::Button;
using ftxui::ButtonOption;
using ftxui::CatchEvent;
using ftxui::center;
using ftxui::Color;
using ftxui::color;
using ftxui::Component;
using ftxui::emptyElement;
using ftxui::Event;
using ftxui::flex_shrink;
using ftxui::hbox;
using ftxui::Input;
using ftxui::Renderer;
using ftxui::ScreenInteractive;
using ftxui::separator;
using ftxui::text;
using ftxui::vbox;

bool ecranConnexion(InfosConnexion& infos, const std::string& erreur_initiale) {
    Component input_pseudo = Input(&infos.pseudo, "Ex: Alice");
    Component input_ip     = Input(&infos.ip, "Ex: 127.0.0.1");
    Component input_port   = Input(&infos.port, "Ex: 4242");

    auto screen = ScreenInteractive::TerminalOutput();
    bool valide = false;
    std::string erreur = erreur_initiale;

    auto on_connect = [&] {
        if (infos.pseudo.empty()) {
            erreur = "Le pseudo ne peut pas etre vide.";
            return;
        }
        valide = true;
        screen.Exit();
    };
    Component btn_connect = Button(
        "Se connecter",
        on_connect,
        ButtonOption::Animated());

    Component container = ftxui::Container::Vertical({
        input_pseudo,
        input_ip,
        input_port,
        btn_connect,
    });

    container |= CatchEvent([&](Event event) {
        if (event == Event::Return) {
            on_connect();
            return true;
        }
        return false;
    });

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

    screen.Loop(renderer);

    return valide;
}
