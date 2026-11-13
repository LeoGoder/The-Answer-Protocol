// Interactive login modal view

#include "../includes/login_view.hpp"

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

/*
 * Displays interactive modal dialog prompting user for connection settings.
 * Validates non-empty pseudo, hooks Enter key to submit and Escape to abort,
 * renders a centered bordered FTXUI dialog, and runs the terminal event loop.
 */
bool showLoginView(ConnectionConfig& config,
                   const std::string& initial_error) {
    // Input components for configuration fields
    Component input_pseudo = Input(&config.pseudo, "Ex: Alice");
    Component input_ip     = Input(&config.ip, "Ex: 127.0.0.1");
    Component input_port   = Input(&config.port, "Ex: 4242");

    auto screen = ScreenInteractive::TerminalOutput();
    bool is_valid = false;
    std::string error = initial_error;

    // Callback when connection is attempted
    auto on_connect = [&] {
        if (config.pseudo.empty()) {
            error = "Pseudo cannot be empty.";
            return;
        }
        is_valid = true;
        screen.Exit();
    };

    // Connect button with animation option
    Component btn_connect = Button(
        "Connect",
        on_connect,
        ButtonOption::Animated());

    // Vertical layout container grouping form controls
    Component container = ftxui::Container::Vertical({
        input_pseudo,
        input_ip,
        input_port,
        btn_connect,
    });

    // Keyboard shortcut handler (Enter to submit, Escape to cancel)
    container |= CatchEvent([&](Event event) {
        if (event == Event::Return) {
            on_connect();
            return true;
        }
        if (event == Event::Escape) {
            screen.Exit();
            return true;
        }
        return false;
    });

    // Render the styled bordered dialog
    Component renderer = Renderer(container, [&] {
        return vbox({
            text(" THE ANSWER PROTOCOL - TUI ") | bold | center,
            separator(),
            hbox({text(" Pseudo : "), input_pseudo->Render()}),
            hbox({text(" IP     : "), input_ip->Render()}),
            hbox({text(" Port   : "), input_port->Render()}),
            separator(),
            error.empty() ? emptyElement()
                          : text(" " + error) | color(Color::Red),
            btn_connect->Render() | center,
        }) | border | flex_shrink;
    });

    // Run event loop until confirmation or cancel
    screen.Loop(renderer);

    return is_valid;
}
