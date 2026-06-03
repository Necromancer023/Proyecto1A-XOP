#define _CRT_SECURE_NO_WARNINGS
#include "item.h"
#include "player.h"
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>

// ================================
// VARIABLE GLOBAL
// ================================
Item item_pool[MAX_ITEMS];

// ================================
// INICIALIZAR POOL
// ================================
void init_item_pool() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        item_pool[i].active = 0;
        item_pool[i].type = 0;
        item_pool[i].value = 0;
    }
}

// ================================
// HELPER INTERNO: busca slot libre
// ================================
static int alloc_item() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!item_pool[i].active) return i;
    }
    return -1;
}

// ================================
// SPAWNEAR MONEDA DAL
// ================================
void spawn_dal(float x, float y) {
    int i = alloc_item();
    if (i == -1) return;

    item_pool[i].x = x;
    item_pool[i].y = y;
    item_pool[i].active = 1;
    item_pool[i].type = 0;      // DAL coin
    item_pool[i].value = 50;     // valor base
    item_pool[i].anim_frame = 0;
    item_pool[i].anim_timer = 0;
}

// ================================
// SPAWNEAR REFLECT SHIELD SLOT
// El jugador gana una bomba extra al recogerla
// ================================
void spawn_bomb_slot(float x, float y) {
    int i = alloc_item();
    if (i == -1) return;

    item_pool[i].x = x;
    item_pool[i].y = y;
    item_pool[i].active = 1;
    item_pool[i].type = 1;      // Reflect Shield Slot
    item_pool[i].value = 1;      // da 1 bomba
}

// ================================
// ACTUALIZAR ITEMS
// ================================
void update_items() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!item_pool[i].active) continue;

        // avanzar animacion cada 6 frames
        if (item_pool[i].type == 0) {
            item_pool[i].anim_timer++;
            if (item_pool[i].anim_timer >= 6) {
                item_pool[i].anim_timer = 0;
                item_pool[i].anim_frame = (item_pool[i].anim_frame + 1) % 8;
            }
        }

        // caer hacia abajo 
        item_pool[i].y += (item_pool[i].type == 1) ? 1.2f : 2.0f;

        // distancia al jugador
        float dx = item_pool[i].x - player.x;
        float dy = item_pool[i].y - player.y;
        float dist = sqrtf(dx * dx + dy * dy);

        // radio de recoleccion mas grande para bombas 
        float pickup_r = (item_pool[i].type == 1) ? 28.0f : 20.0f;

        if (dist <= pickup_r) {

            // sonido de pickup
            if (sfx_pickup)
                al_play_sample(sfx_pickup, 0.7f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, NULL);

            if (item_pool[i].type == 0) {
                // DAL coin: suma puntos y combo
                player.score += item_pool[i].value;
                player.combo++;
            }
            else {
                // Reflect Shield Slot: suma una bomba 9 maximo
                if (player.bombs < 9) player.bombs++;
            }
            item_pool[i].active = 0;
            continue;
        }

        // eliminar si sale de pantalla
        if (item_pool[i].y > SCREEN_H + 10) {
            item_pool[i].active = 0;
            // solo las DAL coins rompen el combo
            if (item_pool[i].type == 0) player.combo = 0;
        }
    }
}

// ================================
// DIBUJAR ITEMS
// DAL coin: circulo amarillo
// Reflect Shield Slot: diamante cian
// ================================
void draw_items() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!item_pool[i].active) continue;

        float x = item_pool[i].x;
        float y = item_pool[i].y;

        if (item_pool[i].type == 0) {
            ALLEGRO_BITMAP* spr = spr_coin[item_pool[i].anim_frame];
            if (spr) {
                int w = al_get_bitmap_width(spr);
                int h = al_get_bitmap_height(spr);
                al_draw_bitmap(spr, x - w / 2.0f, y - h / 2.0f, 0);
            }
            else {
                // fallback geometrico
                al_draw_filled_circle(x, y, 6, al_map_rgb(255, 220, 0));
                al_draw_circle(x, y, 6, al_map_rgb(255, 255, 100), 1.0f);
            }
        }
        else {
            // Reflect Shield Slot:  (mas visible)
            al_draw_filled_triangle(x, y - 9,
                x - 7, y,
                x, y + 9,
                al_map_rgb(0, 220, 255));
            al_draw_filled_triangle(x, y - 9,
                x + 7, y,
                x, y + 9,
                al_map_rgb(0, 220, 255));
            al_draw_triangle(x, y - 9,
                x - 7, y,
                x + 7, y,
                al_map_rgb(150, 255, 255), 1.0f);
            // etiqueta pequeña
            
        }
    }
}