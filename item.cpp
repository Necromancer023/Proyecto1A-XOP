#define _CRT_SECURE_NO_WARNINGS
#include "item.h"
#include "player.h"
#include <allegro5/allegro_primitives.h>
#include <math.h>

// ================================
// VARIABLE GLOBAL
// ================================
Item item_pool[MAX_ITEMS];

// ================================
// INICIALIZAR POOL
// marca todos los slots como inactivos
// ================================
void init_item_pool() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        item_pool[i].active = 0;
    }
}

// ================================
// SPAWNEAR MONEDA DAL
// busca primer slot inactivo y
// configura la moneda en esa posicion
// ================================
void spawn_dal(float x, float y) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (item_pool[i].active) continue;

        item_pool[i].x = x;
        item_pool[i].y = y;
        item_pool[i].active = 1;
        item_pool[i].value = 50;   // valor base de cada moneda DAL
        return;
    }
}

// ================================
// ACTUALIZAR ITEMS
// mueve las monedas hacia abajo,
// detecta si el jugador las recoge
// y elimina las que salen de pantalla
// ================================
void update_items() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!item_pool[i].active) continue;

        // caer hacia abajo
        item_pool[i].y += 2.0f;

        // verificar si el jugador la recoge
        float dx = item_pool[i].x - player.x;
        float dy = item_pool[i].y - player.y;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist <= 20.0f) {
            // recoger moneda
            player.score += item_pool[i].value;
            player.combo++;
            item_pool[i].active = 0;
            continue;
        }

        // eliminar si sale de pantalla — pierde combo
        if (item_pool[i].y > SCREEN_H + 10) {
            item_pool[i].active = 0;
            player.combo = 0;   // perder combo si no la recoge
        }
    }
}

// ================================
// DIBUJAR ITEMS
// moneda amarilla pequeña —
// reemplazar con sprite despues
// ================================
void draw_items() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!item_pool[i].active) continue;

        al_draw_filled_circle(
            item_pool[i].x, item_pool[i].y,
            6, al_map_rgb(255, 220, 0)  // amarillo
        );

        // borde para que se vea mejor
        al_draw_circle(
            item_pool[i].x, item_pool[i].y,
            6, al_map_rgb(255, 255, 100), 1.0f
        );
    }
}