#ifndef COLLISION_H
#define COLLISION_H

#include "game.h"

// ================================
// FUNCIONES
// ================================
int  circles_collide(float x1, float y1, float r1,
    float x2, float y2, float r2);
void check_player_hit();
void check_enemy_hit();

#endif