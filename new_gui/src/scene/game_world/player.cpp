#include "player.hpp"
#include "helper.hpp"
#include "raylib.h"

Player::Player(Texture2D *font, std::string name, Texture2D *sprite) {
    this->player_pos = {30.0, 30.0};
    this->font = font;
    this->name = name;
    this->sprite = sprite;
}

Player::Player() {

}

Player::~Player() {

}

void Player::draw_player(std::string player_name) {
    float player_center = (float(this->sprite->width));
    float text_len = float(GetFontTextWidth(player_name) / 2) + (player_center / 2);

    DrawFontText(*this->font, player_name, this->player_pos.x - text_len + player_center, this->player_pos.y - 20);
    DrawTexture(*this->sprite, this->player_pos.x, this->player_pos.y, WHITE);
}

void Player::input_player(float dt) {
    if (IsKeyDown(KEY_A)) {
        this->player_pos.x -= speed * dt;
    }
    if (IsKeyDown(KEY_D)) {
        this->player_pos.x += speed * dt;
    }
    if (IsKeyDown(KEY_W)) {
        this->player_pos.y -= speed * dt;
    }
    if (IsKeyDown(KEY_S)) {
        this->player_pos.y += speed * dt;
    }
    if (IsKeyPressed(KEY_E)) {
        //
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        // Pause menu enable
    }
}
