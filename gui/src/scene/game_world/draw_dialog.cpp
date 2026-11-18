#include <algorithm>
#include <string>
#include <vector>

#include "inc/game_world.hpp"
#include "inc/helper.hpp"
#include "inc/raylib.h"

Rectangle GameWorld::dialog_rect() {
    return Rectangle{static_cast<float>(SCREEN_WIDTH - DIALOG_WIDTH) / 2,
                     static_cast<float>(SCREEN_HEIGHT - DIALOG_HEIGHT) / 2,
                     DIALOG_WIDTH, DIALOG_HEIGHT};
}

// TALK replies "OK <text>": read before kill_return_line, as the bare "OK" of
// POSITION arrives in the same buffer.
void GameWorld::read_talk_reply(std::string buffer) {
    std::vector<std::string> lines = split(buffer, '\n');

    for (size_t i = 0; i < lines.size(); i++) {
        if (lines[i].rfind("ERR", 0) == 0)
            this->wait_talk = false;
        if (lines[i].rfind("OK ", 0) == 0 && lines[i][3] != '{' &&
            lines[i][3] != '[') {
            this->dialog_text = lines[i].substr(3);
            this->wait_talk = false;
        }
        if (!this->wait_talk)
            return;
    }
}

void GameWorld::draw_dialog() {
    if (this->dialog_npc.empty())
        return;
    // The dialog closes when its NPC leaves the view (room change, defeat) or
    // on a click outside it (not on another NPC).
    bool npc_gone = std::find(this->room.npcs.begin(), this->room.npcs.end(),
                              this->dialog_npc) == this->room.npcs.end();
    bool click_outside = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                         this->hovered_npc.empty() &&
                         !CheckCollisionPointRec(get_virtual_mouse_position(),
                                                 GameWorld::dialog_rect());

    if (npc_gone || click_outside) {
        this->dialog_npc.clear();
        return;
    }
    std::vector<std::string> names = {"TALK"};
    Rectangle panel = GameWorld::dialog_rect();
    int button_y = panel.y + DIALOG_HEIGHT - HUD_PADDING - BUTTON_HEIGHT;

    if (std::find(this->quest_givers.begin(), this->quest_givers.end(),
                  this->dialog_npc) != this->quest_givers.end())
        names.push_back("QUEST");
    if (std::find(this->hostile.begin(), this->hostile.end(),
                  this->dialog_npc) != this->hostile.end())
        names.push_back("ATTACK");
    names.push_back("CLOSE");
    DrawRectangleRec(panel, ColorAlpha(BLACK, 0.8));
    DrawText(GameWorld::display_name(this->dialog_npc).c_str(),
             panel.x + HUD_PADDING, panel.y + HUD_PADDING, HUD_FONT_SIZE,
             YELLOW);
    for (size_t i = 0; i * DIALOG_LINE_CHARS < this->dialog_text.size(); i++)
        DrawText(
            this->dialog_text.substr(i * DIALOG_LINE_CHARS, DIALOG_LINE_CHARS)
                .c_str(),
            panel.x + HUD_PADDING,
            panel.y + HUD_PADDING + (i + 1) * LOG_LINE_HEIGHT, HUD_FONT_SIZE,
            WHITE);

    std::string clicked = "";
    int index =
        GameWorld::draw_text_buttons(names, panel.x + HUD_PADDING, button_y);

    if (index >= 0)
        clicked = names[index];
    if (clicked == "TALK") {
        this->wait_talk = true;
        send_message(this->command.TALK, this->dialog_npc, this->fd);
    }
    if (clicked == "QUEST")
        send_message(this->command.QUEST, this->dialog_npc, this->fd);
    if (clicked == "ATTACK")
        send_message(this->command.ATTACK, this->dialog_npc, this->fd);
    if (clicked == "CLOSE")
        this->dialog_npc.clear();
}
