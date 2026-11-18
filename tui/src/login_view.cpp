// Interactive login modal view

#include "../includes/login_view.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>

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
using ftxui::Element;
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
 * Validates non-empty pseudo, connects in-modal with live status indicator,
 * hooks Enter key to submit, Escape to cancel attempt or abort dialog,
 * and renders a centered bordered FTXUI dialog.
 */
bool showLoginView(NetworkClient& client,
                   ConnectionConfig& config,
                   const std::string& initial_error) {
    // Input components for configuration fields
    Component input_pseudo = Input(&config.pseudo, "Ex: Alice");
    Component input_ip     = Input(&config.ip, "Ex: 127.0.0.1");
    Component input_port   = Input(&config.port, "Ex: 4242");

    auto screen = ScreenInteractive::TerminalOutput();
    screen.ForceHandleCtrlC(false);
    bool is_valid = false;
    std::string error = initial_error;
    std::atomic<bool> is_connecting{false};
    std::unique_ptr<std::thread> connect_thread;

    auto cleanup_thread = [&]() {
        if (connect_thread && connect_thread->joinable()) {
            connect_thread->join();
        }
        connect_thread.reset();
    };

    // Callback when connection is attempted
    auto on_connect = [&] {
        if (is_connecting.load()) return;
        if (config.pseudo.empty()) {
            error = "Pseudo cannot be empty.";
            return;
        }
        error.clear();
        is_connecting.store(true);

        cleanup_thread();

        connect_thread = std::make_unique<std::thread>([&]() {
            std::string err_msg;
            bool ok = client.connectToServer(config, err_msg);
            if (ok) {
                is_valid = true;
                is_connecting.store(false);
                screen.Exit();
            } else {
                if (err_msg.empty()) {
                    error = "Failed to connect to server.";
                } else {
                    error = err_msg;
                }
                is_connecting.store(false);
                screen.PostEvent(Event::Custom);
            }
        });
    };

    // Callback when user requests cancellation
    auto on_cancel = [&] {
        if (is_connecting.load()) {
            client.disconnect();
            cleanup_thread();
            is_connecting.store(false);
            error = "Connection canceled.";
            screen.PostEvent(Event::Custom);
            return;
        }
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

    // Keyboard shortcut handler
    container |= CatchEvent([&](Event event) {
        if (is_connecting.load()) {
            if (event == Event::Escape || event == Event::CtrlC) {
                on_cancel();
                return true;
            }
            return true;  // Consume all other inputs while connecting
        }
        if (event == Event::Return) {
            on_connect();
            return true;
        }
        if (event == Event::Escape || event == Event::CtrlC) {
            screen.Exit();
            return true;
        }
        return false;
    });

    // Render the styled bordered dialog
    Component renderer = Renderer(container, [&] {
        Element action_element;
        if (is_connecting.load()) {
            action_element = hbox({
                text(" Connecting to server... ") | color(Color::Yellow),
                text("(Esc to cancel)") | color(Color::GrayDark)
            }) | center;
        } else {
            action_element = btn_connect->Render() | center;
        }

        return vbox({
            text(" THE ANSWER PROTOCOL - TUI ") | bold | center,
            separator(),
            hbox({text(" Pseudo : "), input_pseudo->Render()}),
            hbox({text(" IP     : "), input_ip->Render()}),
            hbox({text(" Port   : "), input_port->Render()}),
            separator(),
            (error.empty() || is_connecting.load())
                ? emptyElement()
                : text(" " + error) | color(Color::Red),
            action_element,
        }) | border | flex_shrink;
    });

    // Run event loop until confirmation or cancel
    screen.Loop(renderer);

    cleanup_thread();

    return is_valid;
}
