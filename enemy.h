#ifndef ENEMY_H
#define ENEMY_H

#include "game.h"

// ================================
// VARIABLES GLOBALES
// ================================
extern Enemy enemy_pool[MAX_ENEMIES];

// ================================
// FUNCIONES
// ================================
void init_enemy_pool();
void spawn_enemy(float x, float y, float angle, float speed, int hp, int type);
void update_enemies();
void draw_enemies();
void enemy_die(int idx);

#endif