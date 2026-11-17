#include "game_world.hpp"
#include "helper.hpp"
#include "raylib.h"
#include <algorithm>
#include <string>
#include <vector>

void GameWorld::add_log(std::string msg, int tab) {
    std::vector<std::string> &lines = this->panel_lines[tab];

    for (size_t i = 0; i < msg.size(); i += LOG_LINE_CHARS)
        lines.push_back(msg.substr(i, LOG_LINE_CHARS));
    while (lines.size() > LOG_HISTORY)
        lines.erase(lines.begin());
}

// Bottom-left panel: LOG / GROUP / CHAT tabs above it, the lines of the tab (of
// the channel for CHAT), its buttons at the bottom.
void GameWorld::draw_log() {
    Vector2 mouse = get_virtual_mouse_position();
    int log_height =
        LOG_LINES * LOG_LINE_HEIGHT + BUTTON_HEIGHT + HUD_PADDING * 3;
    int log_y = SCREEN_HEIGHT - 10 - log_height;
    int text_x = 10 + HUD_PADDING;
    int button_y = log_y + log_height - HUD_PADDING - BUTTON_HEIGHT;

    DrawRectangle(10, log_y, LOG_WIDTH, log_height, ColorAlpha(BLACK, 0.6));
    int tab = GameWorld::draw_text_buttons(
        {"LOG", "GROUP", "CHAT"}, 10, log_y - BUTTON_HEIGHT, this->panel_tab);

    if (tab >= 0 && tab != this->panel_tab) {
        this->panel_tab = tab;
        this->log_scroll = 0;
    }
    if (this->panel_tab == PANEL_GROUP)
        this->group_unread = false;
    if (this->panel_tab == PANEL_CHAT)
        this->chat_unread[this->player.chat.chat] = false;
    float group_right = 10 + MeasureText("LOG", HUD_FONT_SIZE) +
                        MeasureText("GROUP", HUD_FONT_SIZE) +
                        BUTTON_PADDING * 4 + BUTTON_SPACING;
    float chat_right = group_right + BUTTON_SPACING +
                       MeasureText("CHAT", HUD_FONT_SIZE) + BUTTON_PADDING * 2;

    if (this->group_unread)
        GameWorld::draw_bubble(group_right, log_y - BUTTON_HEIGHT);
    if (this->chat_unread[0] || this->chat_unread[1] || this->chat_unread[2])
        GameWorld::draw_bubble(chat_right, log_y - BUTTON_HEIGHT);
    int lines_index = this->panel_tab == PANEL_CHAT
                          ? PANEL_CHAT + this->player.chat.chat
                          : this->panel_tab;
    std::vector<std::string> &lines = this->panel_lines[lines_index];
    std::string header = "";
    if (this->panel_tab == PANEL_GROUP && !this->group_members.empty()) {
        header = "Leader: " + this->group_members[0] +
                 "  |  Members: " + this->group_members[0];
        for (size_t i = 1; i < this->group_members.size(); i++)
            header += ", " + this->group_members[i];
    }
    int shown = header.empty() ? LOG_LINES : LOG_LINES - 1;
    int lines_y = header.empty() ? log_y + HUD_PADDING
                                 : log_y + HUD_PADDING + LOG_LINE_HEIGHT;
    int max_scroll = std::max(0, int(lines.size()) - shown);
    int first;

    if (CheckCollisionPointRec(
            mouse, Rectangle{10, float(log_y), LOG_WIDTH, float(log_height)}))
        this->log_scroll += int(GetMouseWheelMove());
    this->log_scroll = std::clamp(this->log_scroll, 0, max_scroll);
    first = max_scroll - this->log_scroll;
    if (!header.empty())
        DrawText(header.c_str(), text_x, log_y + HUD_PADDING, HUD_FONT_SIZE,
                 YELLOW);
    for (int i = 0; i < shown && first + i < int(lines.size()); i++)
        DrawText(lines[first + i].c_str(), text_x,
                 lines_y + i * LOG_LINE_HEIGHT, HUD_FONT_SIZE, WHITE);
    if (this->panel_tab == PANEL_GROUP) {
        GameWorld::draw_group_buttons(text_x, button_y);
        return;
    }
    if (this->panel_tab == PANEL_CHAT) {
        GameWorld::draw_chat_buttons(text_x, button_y);
        return;
    }
    std::string commands[4] = {this->command.LOOK, this->command.STATUS,
                               this->command.QUESTS, this->command.WHO};
    int clicked = GameWorld::draw_text_buttons(
        {"LOOK", "STATUS", "QUESTS", "WHO"}, text_x, button_y);

    if (clicked == 2) {
        this->wait_quests = true;
        this->log_quests = true;
    }
    if (clicked >= 0)
        send_message(commands[clicked], "", this->fd);
}

// Outside a group: CREATE, and JOIN <leader> after an invitation. Inside: LEAVE
// and INVITE, which lists the other players of the room.
void GameWorld::draw_group_buttons(int x, int y) {
    std::vector<std::string> names;

    if (this->group_members.empty()) {
        names.push_back("CREATE");
        if (!this->group_inviter.empty())
            names.push_back("JOIN " + this->group_inviter);
    }
    else if (!this->inviting) {
        names.push_back("LEAVE");
        names.push_back("INVITE");
    }
    else {
        for (size_t i = 0; i < this->room.player.size(); i++) {
            std::string name = this->room.player[i]->name;

            if (std::find(this->group_members.begin(),
                          this->group_members.end(),
                          name) == this->group_members.end())
                names.push_back(name);
        }
        names.push_back("BACK");
    }
    int clicked = GameWorld::draw_text_buttons(names, x, y);

    if (clicked < 0)
        return;
    std::string name = names[clicked];

    if (name == "CREATE")
        send_message(this->command.GROUP, "CREATE", this->fd);
    else if (name == "LEAVE") {
        send_message(this->command.GROUP, "LEAVE", this->fd);
        this->group_members.clear();
    }
    else if (name == "INVITE")
        this->inviting = true;
    else if (name == "BACK")
        this->inviting = false;
    else if (this->inviting) {
        send_message(this->command.GROUP, "INVITE " + name, this->fd);
        GameWorld::add_log("Invitation sent to " + name, PANEL_GROUP);
        this->inviting = false;
    }
    else {
        send_message(this->command.GROUP, "JOIN " + this->group_inviter,
                     this->fd);
        this->group_inviter.clear();
    }
}

// CHAT tab: GLOBAL / ROOM / GROUP channel buttons, then the input box (click on
// it or press "/" to type).
void GameWorld::draw_chat_buttons(int x, int y) {
    std::vector<std::string> names = {"GLOBAL", "ROOM", "GROUP"};
    int clicked =
        GameWorld::draw_text_buttons(names, x, y, this->player.chat.chat);
    float box_x = x;

    for (size_t i = 0; i < names.size(); i++) {
        box_x +=
            MeasureText(names[i].c_str(), HUD_FONT_SIZE) + BUTTON_PADDING * 2;
        if (this->chat_unread[i])
            GameWorld::draw_bubble(box_x, y);
        box_x += BUTTON_SPACING;
    }
    if (clicked >= 0) {
        this->player.chat.chat = ChatService(clicked);
        this->chat_unread[clicked] = false;
        this->log_scroll = 0;
    }
    Rectangle box = {box_x, float(y), 10 + LOG_WIDTH - HUD_PADDING - box_x,
                     BUTTON_HEIGHT};
    std::string text =
        this->player.chat.get_msg() + (this->player.show_chat ? "_" : "");

    while (MeasureText(text.c_str(), HUD_FONT_SIZE) >
           box.width - BUTTON_PADDING * 2)
        text.erase(0, 1);
    DrawRectangleRec(box, ColorAlpha(BLACK, 0.6));
    DrawRectangleLinesEx(box, 1, this->player.show_chat ? YELLOW : GRAY);
    if (text.empty())
        DrawText("click or press / to type", box.x + BUTTON_PADDING,
                 y + BUTTON_PADDING, HUD_FONT_SIZE, GRAY);
    else
        DrawText(text.c_str(), box.x + BUTTON_PADDING, y + BUTTON_PADDING,
                 HUD_FONT_SIZE, WHITE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(get_virtual_mouse_position(), box))
        this->player.show_chat = true;
}

// "/": CHAT tab on the first channel with unread messages (GLOBAL, ROOM,
// GROUP), GLOBAL if none, and start typing.
void GameWorld::open_chat() {
    this->player.chat.chat = global;
    for (int i = 2; i >= 0; i--) {
        if (this->chat_unread[i])
            this->player.chat.chat = ChatService(i);
    }
    this->panel_tab = PANEL_CHAT;
    this->log_scroll = 0;
    this->player.show_chat = true;
}

// Sends the typed message to the current channel; GROUP without a group stays
// local (the server error would only say 401).
void GameWorld::send_chat() {
    const std::string scopes[3] = {"GLOBAL", "ROOM", "GROUP"};
    std::string msg = this->player.chat.get_msg();
    int channel = this->player.chat.chat;

    this->player.chat.clear_msg();
    if (msg.empty())
        return;
    if (channel == group && this->group_members.empty())
        GameWorld::add_log(
            "You are not in a group: create or join one in the GROUP tab.",
            PANEL_CHAT + channel);
    else
        send_message(this->command.CHAT, scopes[channel] + " " + msg, this->fd);
}
