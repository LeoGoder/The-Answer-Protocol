#include "game_world.hpp"
#include "helper.hpp"
#include "raylib.h"
#include <algorithm>
#include <string>

void GameWorld::add_log(std::string msg) {
    for (size_t i = 0; i < msg.size(); i += LOG_LINE_CHARS)
        this->log.push_back(msg.substr(i, LOG_LINE_CHARS));
    while (this->log.size() > LOG_HISTORY)
        this->log.erase(this->log.begin());
}

void GameWorld::draw_log() {
    std::string commands[4] = {this->command.LOOK, this->command.STATUS, this->command.QUESTS, this->command.WHO};
    Vector2 mouse = get_virtual_mouse_position();
    int count_height = HUD_FONT_SIZE + HUD_PADDING * 2;
    int log_height = (LOG_LINES + 1) * LOG_LINE_HEIGHT + HUD_PADDING * 3;
    int log_y = SCREEN_HEIGHT - 10 - count_height - 4 - log_height;
    int button_x = 10 + HUD_PADDING;
    int button_y = log_y + log_height - HUD_PADDING - HUD_FONT_SIZE;

    int max_scroll = std::max(0, int(this->log.size()) - LOG_LINES);
    int first;

    if (CheckCollisionPointRec(mouse, Rectangle{10, float(log_y), LOG_WIDTH, float(log_height)}))
        this->log_scroll += int(GetMouseWheelMove());
    this->log_scroll = std::clamp(this->log_scroll, 0, max_scroll);
    first = max_scroll - this->log_scroll;
    DrawRectangle(10, log_y, LOG_WIDTH, log_height, ColorAlpha(BLACK, 0.6));
    for (int i = 0; i < LOG_LINES && first + i < int(this->log.size()); i++)
        DrawText(this->log[first + i].c_str(), 10 + HUD_PADDING, log_y + HUD_PADDING + i * LOG_LINE_HEIGHT, HUD_FONT_SIZE, WHITE);
    DrawLine(10, button_y - HUD_PADDING, 10 + LOG_WIDTH, button_y - HUD_PADDING, GRAY);
    int clicked = GameWorld::draw_text_buttons({"LOOK", "STATUS", "QUESTS", "WHO"}, button_x, button_y, log_y + log_height);

    if (clicked == 2) {
        this->wait_quests = true;
        this->log_quests = true;
    }
    if (clicked >= 0)
        send_message(commands[clicked], "", this->fd);
}

// Returns the index of the clicked button, -1 if none.
int GameWorld::draw_text_buttons(std::vector<std::string> names, int x, int y, int bottom) {
    Vector2 mouse = get_virtual_mouse_position();
    int clicked = -1;

    for (size_t i = 0; i < names.size(); i++) {
        int width = MeasureText(names[i].c_str(), HUD_FONT_SIZE);
        bool hovered = CheckCollisionPointRec(mouse, Rectangle{float(x), float(y), float(width), HUD_FONT_SIZE});

        DrawText(names[i].c_str(), x, y, HUD_FONT_SIZE, hovered ? YELLOW : WHITE);
        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            clicked = i;
        x += width + LOG_BUTTON_SPACING;
        if (i + 1 < names.size())
            DrawLine(x - LOG_BUTTON_SPACING / 2, y - HUD_PADDING, x - LOG_BUTTON_SPACING / 2, bottom, GRAY);
    }
    return clicked;
}
