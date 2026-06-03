#ifndef ITEM_H
#define ITEM_H

#include "game.h"

// ================================
// POOL DE ITEMS
// type 0 = DAL coin (puntos + combo)
// type 1 = Reflect Shield Slot (bomba extra)
// ================================
extern Item item_pool[MAX_ITEMS];

// ================================
// FUNCIONES
// ================================
void init_item_pool();
void spawn_dal(float x, float y);
void spawn_bomb_slot(float x, float y);   
void update_items();
void draw_items();

#endif