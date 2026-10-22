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
using ftxui::xflex;
using ftxui::yframe;

void runGameView(NetworkClient& client,
                 AppState& state,
                 const ConnectionConfig& config) {
    state.setUserPseudo(config.pseudo);

    auto screen = ScreenInteractive::Fullscreen();

    std::string saisie;
    int cursor_pos = 0;
    size_t history_index = 0;
    std::string current_draft;

    ftxui::InputOption input_option;
    input_option.content = &saisie;
    input_option.placeholder = "Tapez votre commande ici...";
    input_option.multiline = false;
    input_option.cursor_position = &cursor_pos;
    Component champ = Input(input_option);

    Component renderer = Renderer(champ, [&] {
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
        if (!msg_elements.empty()) {
            msg_elements.back() = msg_elements.back() | focus;
        }

        std::string title = " Terminal - " + config.pseudo + " @ " +
                            config.ip + ":" + config.port + " ";

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

        Elements quest_elements;
        std::vector<QuestInfo> quests = state.getActiveQuests();
        if (quests.empty()) {
            quest_elements.push_back(
                shrinkable(paragraph("  No active quests")) |
                    color(Color::GrayDark));
        } else {
            for (const auto& q : quests) {
                std::string title =
                    !q.description.empty() ? q.description : q.quest_id;
                if (title.empty()) title = "Unknown quest";
                quest_elements.push_back(
                    shrinkable(paragraph("  " + title)) |
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

        int pos = std::max(0, std::min(cursor_pos,
                                       static_cast<int>(saisie.size())));
        Elements input_parts;
        input_parts.push_back(text(" > ") | color(Color::Cyan) | bold);

        if (saisie.empty()) {
            input_parts.push_back(text("█") | color(Color::Cyan));
            input_parts.push_back(
                text(" Tapez votre commande ici...") | color(Color::GrayDark));
        } else if (pos < static_cast<int>(saisie.size())) {
            std::string before = saisie.substr(0, pos);
            std::string at_char = saisie.substr(pos, 1);
            std::string after = saisie.substr(pos + 1);
            if (!before.empty()) {
                input_parts.push_back(text(before));
            }
            input_parts.push_back(
                text(at_char) | bgcolor(Color::Cyan) |
                color(Color::Black) | bold);
            if (!after.empty()) {
                input_parts.push_back(text(after));
            }
        } else {
            input_parts.push_back(text(saisie));
            input_parts.push_back(text("█") | color(Color::Cyan));
        }

        input_parts.push_back(emptyElement() | xflex);
        std::string pos_str = " [" + std::to_string(pos) + "/" +
                              std::to_string(saisie.size()) + "] ";
        input_parts.push_back(text(pos_str) | color(Color::GrayLight));

        Element terminal_panel = vbox({
            window(text(title),
                   vbox(msg_elements) | yframe | flex) | flex,
            borderRounded(hbox(std::move(input_parts))),
        }) | flex;

        std::string map_path = state.getMapImage();
        Element map_panel =
            window(text(" Map "),
                   renderImage(map_path, 0, 0, false) | flex) | flex;

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

        return hbox({
            left_column,
            terminal_panel | flex,
            right_column,
        });
    });

    renderer |= CatchEvent([&](Event event) {
        if (event == Event::ArrowUp) {
            std::vector<std::string> history = state.getCommandHistory();
            if (history.empty()) return true;
            if (history_index == history.size()) {
                current_draft = saisie;
            }
            if (history_index > 0) {
                history_index--;
                saisie = history[history_index];
                cursor_pos = static_cast<int>(saisie.size());
            }
            return true;
        }

        if (event == Event::ArrowDown) {
            std::vector<std::string> history = state.getCommandHistory();
            if (history_index < history.size()) {
                history_index++;
                if (history_index == history.size()) {
                    saisie = current_draft;
                } else {
                    saisie = history[history_index];
                }
                cursor_pos = static_cast<int>(saisie.size());
            }
            return true;
        }

        if (event != Event::Return) return false;

        if (saisie == "QUIT") {
            screen.Exit();
            return true;
        }
        if (!saisie.empty()) {
            state.addCommandHistory(saisie);
            history_index = state.getCommandHistory().size();
            current_draft.clear();
            cursor_pos = 0;

            std::string cmd_upper = saisie;
            for (auto& c : cmd_upper) {
                c = static_cast<char>(std::toupper(c));
            }

            std::string trimmed = cmd_upper;
            while (!trimmed.empty() &&
                   std::isspace(static_cast<unsigned char>(trimmed.front()))) {
                trimmed.erase(trimmed.begin());
            }
            while (!trimmed.empty() &&
                   std::isspace(static_cast<unsigned char>(trimmed.back()))) {
                trimmed.pop_back();
            }

            if (trimmed == "QUIT") {
                screen.Exit();
                return true;
            }
            if (trimmed == "CLEAN" || trimmed == "CLEAR" ||
                trimmed == "/CLEAN" || trimmed == "/CLEAR") {
                state.clearMessages();
                state.clearCompletedQuests();
                saisie.clear();
                return true;
            }
            if (cmd_upper == "LOOK") {
                state.setPendingCommand(CMD_LOOK);
            } else if (cmd_upper == "MOVE" ||
                       cmd_upper.rfind("MOVE ", 0) == 0) {
                state.setPendingCommand(CMD_MOVE);
            } else if (cmd_upper.rfind("QUEST ", 0) == 0) {
                state.setPendingCommand(CMD_QUEST);
            } else if (cmd_upper == "QUESTS") {
                state.setPendingCommand(CMD_QUESTS);
            } else if (cmd_upper == "TAKE" ||
                       cmd_upper.rfind("TAKE ", 0) == 0) {
                state.setPendingCommand(CMD_TAKE);
            } else if (cmd_upper == "DROP" ||
                       cmd_upper.rfind("DROP ", 0) == 0) {
                state.setPendingCommand(CMD_DROP);
            } else if (cmd_upper == "INVENTORY") {
                state.setPendingCommand(CMD_INVENTORY);
            } else if (cmd_upper == "STATUS" ||
                       cmd_upper.rfind("TALK ", 0) == 0) {
                state.setPendingCommand(CMD_OTHER_JSON);
            }

            ParsedChat chat = parseChatCommand(saisie);
            if (chat.is_chat) {
                state.pushPendingChat({config.pseudo, chat.message,
                                       chat.channel, saisie});
                client.sendLine(chat.command_to_send);
            } else {
                state.addMessage(config.pseudo, saisie, true);
                client.sendLine(saisie);
            }
            saisie.clear();
        }
        return true;
    });

    client.startListener(screen, state);
    screen.Loop(renderer);

    state.setInterfaceActive(false);
    client.disconnect();
}
