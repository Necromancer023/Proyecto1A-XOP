#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

// ================================
// VARIABLE GLOBAL DEL JUGADOR
// ================================
extern Player player;

// ================================
// FUNCIONES
// ================================
void init_player();
void update_player();
void draw_player();
void player_shoot();
void player_reflect_shield();
void player_take_damage();
void player_change_weapon();

#endif