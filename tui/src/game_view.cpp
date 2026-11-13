/**
 * @file game_view.cpp
 * @brief Main interactive game view implementation using FTXUI.
 *
 * Provides the fullscreen terminal UI layout comprising:
 * - Left column: Inventory items and active quests.
 * - Center column: Server/system message log and interactive command input bar.
 * - Right column: Dynamic ASCII/pixel room map and multi-channel chat log.
 * Also handles keyboard navigation, command history, and protocol command dispatching.
 */

#include "../includes/game_view.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "../includes/image_renderer.hpp"
#include "../includes/protocol.hpp"
#include "../includes/ui_helpers.hpp"

using ftxui::bgcolor;
using ftxui::bold;
using ftxui::borderRounded;
using ftxui::CatchEvent;
using ftxui::Color;
using ftxui::color;
using ftxui::Component;
using ftxui::Element;
using ftxui::Elements;
using ftxui::emptyElement;
using ftxui::Event;
using ftxui::flex;
using ftxui::focus;
using ftxui::hbox;
using ftxui::Input;
using ftxui::paragraph;
using ftxui::Renderer;
using ftxui::ScreenInteractive;
using ftxui::text;
using ftxui::vbox;
using ftxui::window;
using ftxui::xframe;
using ftxui::xflex;
using ftxui::yframe;

// Helper mapping uppercase input command verbs to PendingCommand states
static int detectPendingCommand(const std::string& cmd) {
    if (cmd == "LOOK") return CMD_LOOK;
    if (cmd == "QUESTS") return CMD_QUESTS;
    if (cmd == "INVENTORY") return CMD_INVENTORY;
    if (startsWith(cmd, "MOVE")) return CMD_MOVE;
    if (startsWith(cmd, "QUEST ")) return CMD_QUEST;
    if (startsWith(cmd, "TAKE")) return CMD_TAKE;
    if (startsWith(cmd, "DROP")) return CMD_DROP;
    if (startsWith(cmd, "USE")) return CMD_USE;
    if (startsWith(cmd, "ATTACK")) return CMD_ATTACK;
    if (cmd == "STATUS" || startsWith(cmd, "TALK ")) return CMD_OTHER_JSON;
    return CMD_NONE;
}

/**
 * @brief Runs the primary interactive game loop.
 *
 * Sets up FTXUI components, attaches input/event handlers, starts the background
 * network listener thread, and enters the interactive event loop until the user
 * quits or connection is severed.
 *
 * @param client Network client instance for socket communication.
 * @param state Shared thread-safe application state.
 * @param config Active connection configuration (IP, port, pseudo).
 * @return True if the loop terminated due to unexpected connection loss, false otherwise.
 */
bool runGameView(NetworkClient& client,
                 AppState& state,
                 const ConnectionConfig& config) {
    state.setUserPseudo(config.pseudo);

    auto screen = ScreenInteractive::Fullscreen();

    std::string user_input;
    int cursor_pos = 0;
    size_t history_index = 0;
    std::string current_draft;

    ftxui::InputOption input_option;
    input_option.content = &user_input;
    input_option.placeholder = "Type your command here...";
    input_option.multiline = false;
    input_option.cursor_position = &cursor_pos;
    Component input_component = Input(input_option);

    Component renderer = Renderer(input_component, [&] {
        // --- 1. Main Terminal Message Log ---
        Elements msg_elements;
        std::vector<ChatMessage> msgs = state.getMessages();
        for (const auto& msg : msgs) {
            if (msg.pseudo.empty()) {
                msg_elements.push_back(
                    wrapLine("", msg.content, Color::White));
            } else if (msg.is_local) {
                msg_elements.push_back(
                    wrapLine(msg.pseudo, msg.content, Color::Cyan));
            } else {
                msg_elements.push_back(
                    wrapLine(msg.pseudo, msg.content, Color::Yellow));
            }
        }
        // Focus the last message to keep auto-scroll anchored to the bottom
        if (!msg_elements.empty()) {
            msg_elements.back() = msg_elements.back() | focus;
        }

        std::string title = " Terminal - " + config.pseudo + " @ " +
                            config.ip + ":" + config.port + " ";

        // --- 2. Inventory Items Panel ---
        Elements item_elements;
        std::vector<std::string> inventory = state.getInventory();
        if (inventory.empty()) {
            item_elements.push_back(
                shrinkable(paragraph("  No items")) |
                    color(Color::GrayDark));
        } else {
            for (const auto& item : inventory) {
                item_elements.push_back(
                    shrinkable(paragraph("  - " + item)) |
                        color(Color::White));
            }
        }
        Element items_panel =
            window(text(" Items "),
                   vbox(item_elements) | yframe | flex) | flex;

        // --- 3. Active Quests Panel ---
        Elements quest_elements;
        std::vector<QuestInfo> quests = state.getActiveQuests();
        if (quests.empty()) {
            quest_elements.push_back(
                shrinkable(paragraph("  No active quests")) |
                    color(Color::GrayDark));
        } else {
            for (const auto& q : quests) {
                std::string quest_title =
                    !q.description.empty() ? q.description : q.quest_id;
                if (quest_title.empty()) quest_title = "Unknown quest";
                quest_elements.push_back(
                    shrinkable(paragraph("  " + quest_title)) |
                        color(Color::Yellow));
                std::string progress_str;
                if (!q.progress.empty()) {
                    progress_str = "    (" + q.progress + ")";
                } else if (q.required_items > 0) {
                    progress_str = "    (Need: " +
                                   std::to_string(q.required_items) + ")";
                }
                if (!q.status.empty()) {
                    if (progress_str.empty()) progress_str = "   ";
                    progress_str += " [" + q.status + "]";
                }
                if (!progress_str.empty()) {
                    quest_elements.push_back(
                        shrinkable(paragraph(progress_str)) |
                            color(Color::GrayLight));
                }
            }
        }
        Element quests_panel =
            window(text(" Quests "),
                   vbox(quest_elements) | yframe | flex) | flex;
        Element left_column =
            vbox({items_panel, quests_panel}) | flex;

        // --- 4. Command Input Box with Custom Cursor ---
        int pos = std::max(0, std::min(cursor_pos,
                                       static_cast<int>(user_input.size())));
        Elements content_parts;

        // Custom cursor rendering: highlight the character under cursor with cyan background
        if (user_input.empty()) {
            content_parts.push_back(text("█") | color(Color::Cyan));
            content_parts.push_back(
                text(" Type your command here...") | color(Color::GrayDark));
        } else if (pos < static_cast<int>(user_input.size())) {
            // Split input into before-cursor, inverted active character, and after-cursor segments
            std::string before = user_input.substr(0, pos);
            std::string at_char = user_input.substr(pos, 1);
            std::string after = user_input.substr(pos + 1);
            if (!before.empty()) {
                content_parts.push_back(text(before));
            }
            content_parts.push_back(
                text(at_char) | bgcolor(Color::Cyan) |
                color(Color::Black) | bold);
            if (!after.empty()) {
                content_parts.push_back(text(after));
            }
        } else {
            // Cursor is at end of input line: draw terminal block cursor
            content_parts.push_back(text(user_input));
            content_parts.push_back(text("█") | color(Color::Cyan));
        }

        std::string pos_str = " [" + std::to_string(pos) + "/" +
                              std::to_string(user_input.size()) + "] ";

        Elements bar_parts;
        bar_parts.push_back(text(" > ") | color(Color::Cyan) | bold);
        bar_parts.push_back(hbox(std::move(content_parts)) | xframe | flex);
        bar_parts.push_back(text(pos_str) | color(Color::GrayLight));

        Element input_box =
            shrinkable(borderRounded(hbox(std::move(bar_parts)))) |
            ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 3);

        Element terminal_panel = vbox({
            window(text(title),
                   vbox(msg_elements) | yframe | flex) | flex,
            input_box,
        }) | flex;

        // --- 5. Map Panel ---
        std::string map_path = state.getMapImage();
        Element map_panel =
            window(text(" Map "),
                   renderImage(map_path, 0, 0, false) | flex) | flex;

        // --- 6. Chat Panel ---
        Elements chat_elements;
        std::vector<ChatMessage> chat_msgs = state.getChatMessages();
        for (const auto& msg : chat_msgs) {
            if (msg.is_local) {
                chat_elements.push_back(
                    wrapLine(msg.pseudo, msg.content,
                             Color::Cyan, msg.channel));
            } else {
                chat_elements.push_back(
                    wrapLine(msg.pseudo, msg.content,
                             Color::Yellow, msg.channel));
            }
        }
        if (!chat_elements.empty()) {
            chat_elements.back() = chat_elements.back() | focus;
        }

        Element chat_panel =
            window(text(" Chat "),
                   vbox(chat_elements) | yframe | flex) | flex;
        Element right_column =
            vbox({map_panel, chat_panel}) | flex;

        // Assemble 3-column layout
        return hbox({
            left_column,
            terminal_panel | flex,
            right_column,
        });
    });

    // Handle keyboard events (history navigation & command dispatch)
    renderer |= CatchEvent([&](Event event) {
        // ArrowUp: Navigate older commands in history
        if (event == Event::ArrowUp) {
            std::vector<std::string> history = state.getCommandHistory();
            if (history.empty()) return true;
            // Stash unsubmitted draft before cycling backward through history
            if (history_index == history.size()) {
                current_draft = user_input;
            }
            if (history_index > 0) {
                history_index--;
                user_input = history[history_index];
                cursor_pos = static_cast<int>(user_input.size());
            }
            return true;
        }

        // ArrowDown: Navigate newer commands or restore current draft
        if (event == Event::ArrowDown) {
            std::vector<std::string> history = state.getCommandHistory();
            if (history_index < history.size()) {
                history_index++;
                // Restore stashed draft when returning to bottom of history
                if (history_index == history.size()) {
                    user_input = current_draft;
                } else {
                    user_input = history[history_index];
                }
                cursor_pos = static_cast<int>(user_input.size());
            }
            return true;
        }

        if (event != Event::Return) return false;

        if (!user_input.empty()) {
            state.addCommandHistory(user_input);
            history_index = state.getCommandHistory().size();
            current_draft.clear();
            cursor_pos = 0;

            std::string trimmed = trim(toUpper(user_input));

            // Client-side quit shortcuts
            if (trimmed == "QUIT" || trimmed == "/QUIT" ||
                trimmed == "EXIT" || trimmed == "/EXIT") {
                client.setUserQuit(true);
                screen.Exit();
                return true;
            }

            // Client-side clear shortcuts
            if (trimmed == "CLEAN" || trimmed == "CLEAR" ||
                trimmed == "/CLEAN" || trimmed == "/CLEAR") {
                state.clearMessages();
                state.clearCompletedQuests();
                user_input.clear();
                return true;
            }

            // Set pending command state for JSON parsing upon response
            int cmd = detectPendingCommand(trimmed);
            if (cmd != CMD_NONE) {
                state.setPendingCommand(cmd);
            }

            // Handle chat commands vs raw game commands
            ParsedChat chat = parseChatCommand(user_input);
            if (chat.is_chat) {
                state.pushPendingChat({config.pseudo, chat.message,
                                       chat.channel, user_input});
                client.sendLine(chat.command_to_send);
            } else {
                state.addMessage(config.pseudo, user_input, true);
                client.sendLine(user_input);
            }
            user_input.clear();
        }
        return true;
    });

    // Start background network listener and run FTXUI loop
    client.startListener(screen, state);
    screen.Loop(renderer);

    // Clean up upon loop exit
    state.setInterfaceActive(false);
    client.disconnect();

    return client.isConnectionLost();
}
