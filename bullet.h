#ifndef BULLET_H
#define BULLET_H

#include "game.h"

// ================================
// VARIABLES GLOBALES DEL POOL
// ================================
extern Bullet bullet_pool[MAX_BULLETS];
extern int free_bullets[MAX_BULLETS];
extern int free_top;

// ================================
// FUNCIONES
// ================================
void init_bullet_pool();
int  alloc_bullet();
void free_bullet(int idx);
void update_bullets();
void draw_bullets();

// patrones de disparo
void fire_single(float x, float y, float angle, float speed, int owner);
void fire_circle(float x, float y, int cantidad, float speed);
void fire_aimed(float ex, float ey, float px, float py, float speed);
void fire_spread(float ex, float ey, float px, float py, float speed, int cantidad);

#endif