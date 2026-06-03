#include "enemy.h"
#include "bullet.h"
#include "player.h"
#include "item.h"
#include "game.h"
#include <math.h>
#include <stdlib.h>
#include <allegro5/allegro_primitives.h>

// ================================
// VARIABLE GLOBAL
// ================================
Enemy enemy_pool[MAX_ENEMIES];

// ================================
// TABLA DE PATRONES DE DISPARO
// Fase 0 = patron normal
// Fase 1 = patron agresivo (jefes en segunda fase)
// ================================
typedef void (*FirePattern)(int idx);

// --- patrones normales (fase 0) ---

void pattern_aimed(int idx) {
    float speed = 3.0f * diff_bullet_speed();
    fire_aimed(enemy_pool[idx].x, enemy_pool[idx].y,
        player.x, player.y, speed);
}

void pattern_circle(int idx) {
    float speed = 2.5f * diff_bullet_speed();
    fire_circle(enemy_pool[idx].x, enemy_pool[idx].y, 12, speed);
}

void pattern_spread(int idx) {
    float speed = 3.0f * diff_bullet_speed();
    fire_spread(enemy_pool[idx].x, enemy_pool[idx].y,
        player.x, player.y, speed, 5);
}

void pattern_double_circle(int idx) {
    float s1 = 2.0f * diff_bullet_speed();
    float s2 = 3.5f * diff_bullet_speed();
    fire_circle(enemy_pool[idx].x, enemy_pool[idx].y, 8, s1);
    fire_circle(enemy_pool[idx].x, enemy_pool[idx].y, 8, s2);
}

void pattern_spiral(int idx) {
    float angle = (enemy_pool[idx].frame_count * 0.15f);
    float speed = 3.0f * diff_bullet_speed();
    fire_single(enemy_pool[idx].x, enemy_pool[idx].y, angle, speed, 1);
}

// --- patrones agresivos (fase 1 de jefes) ---
// Cada patron de fase 1 combina dos patrones base

void pattern_aimed_phase2(int idx) {
    // apuntado triple en abanico cerrado
    float speed = 3.5f * diff_bullet_speed();
    fire_spread(enemy_pool[idx].x, enemy_pool[idx].y,
        player.x, player.y, speed, 3);
}

void pattern_circle_phase2(int idx) {
    // circulo denso + apuntado
    float speed = 3.0f * diff_bullet_speed();
    fire_circle(enemy_pool[idx].x, enemy_pool[idx].y, 20, speed);
    fire_aimed(enemy_pool[idx].x, enemy_pool[idx].y,
        player.x, player.y, speed * 1.3f);
}

void pattern_spread_phase2(int idx) {
    // abanico ancho + circulo lento
    float speed = 3.2f * diff_bullet_speed();
    fire_spread(enemy_pool[idx].x, enemy_pool[idx].y,
        player.x, player.y, speed, 9);
    fire_circle(enemy_pool[idx].x, enemy_pool[idx].y, 8, 1.5f * diff_bullet_speed());
}

void pattern_double_circle_phase2(int idx) {
    // triple circulo a distintas velocidades
    fire_circle(enemy_pool[idx].x, enemy_pool[idx].y, 10, 1.8f * diff_bullet_speed());
    fire_circle(enemy_pool[idx].x, enemy_pool[idx].y, 10, 3.0f * diff_bullet_speed());
    fire_circle(enemy_pool[idx].x, enemy_pool[idx].y, 10, 4.2f * diff_bullet_speed());
}

void pattern_spiral_phase2(int idx) {
    // espiral doble 
    float angle = (enemy_pool[idx].frame_count * 0.18f);
    float speed = 3.5f * diff_bullet_speed();
    fire_single(enemy_pool[idx].x, enemy_pool[idx].y, angle, speed, 1);
    fire_single(enemy_pool[idx].x, enemy_pool[idx].y, angle + 3.14159f, speed, 1);
}

// tabla normal
FirePattern pattern_table[] = {
    pattern_aimed,
    pattern_circle,
    pattern_spread,
    pattern_double_circle,
    pattern_spiral
};

// tabla fase 2
FirePattern pattern_table_phase2[] = {
    pattern_aimed_phase2,
    pattern_circle_phase2,
    pattern_spread_phase2,
    pattern_double_circle_phase2,
    pattern_spiral_phase2
};

#define NUM_PATTERNS 5

// ================================
// INICIALIZAR POOL
// ================================
void init_enemy_pool() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemy_pool[i].active = 0;
    }
}

// ================================
// SPAWNEAR ENEMIGO
// ================================
void spawn_enemy(float x, float y, float angle, float speed, int hp, int type) {
    // modo Massacre: spawnear el doble de enemigos
    int times = (game_mode == MODE_MASSACRE) ? 2 : 1;

    for (int t = 0; t < times; t++) {
        float offset_x = (t == 0) ? 0 : 30.0f;

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemy_pool[i].active) continue;

            int final_hp = hp + diff_enemy_hp_bonus();
            if (final_hp < 1) final_hp = 1;

            int base_fire = 60 + diff_fire_rate_bonus();
            if (base_fire < 15) base_fire = 15;

            enemy_pool[i].x = x + offset_x;
            enemy_pool[i].y = y;
            enemy_pool[i].angle = angle;
            enemy_pool[i].speed = speed;
            enemy_pool[i].hp = final_hp;
            enemy_pool[i].active = 1;
            enemy_pool[i].type = type % NUM_PATTERNS;
            enemy_pool[i].fire_timer = base_fire;
            enemy_pool[i].phase = 0;
            enemy_pool[i].frame_count = 0;
            break;
        }
    }
}

// ================================
// CUANDO MUERE UN ENEMIGO
// 10% de probabilidad de soltar
// un Reflect Shield Slot en lugar
// de solo monedas DAL
// ================================
void enemy_die(int idx) {
    float ex = enemy_pool[idx].x;
    float ey = enemy_pool[idx].y;

    enemy_pool[idx].active = 0;
    player.score += 100 * (1 + player.combo / 10);
    player.combo++;

    // decidir que item dropear
    int roll = rand() % 100;
    if (roll < 10) {
        // 10%: Reflect Shield Slot (bomba extra)
        spawn_bomb_slot(ex, ey);
        // tambien sueltan una moneda
        spawn_dal(ex - 15, ey);
        spawn_dal(ex + 15, ey);
    }
    else {
        // 90%: solo DAL coins
        spawn_dal(ex, ey);
        spawn_dal(ex - 15, ey);
        spawn_dal(ex + 15, ey);
    }
}

// ================================
// ACTUALIZAR ENEMIGOS
// Detecta transicion a fase 2
// cuando el jefe pierde la mitad
// de su vida
// ================================
void update_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemy_pool[i].active) continue;

        enemy_pool[i].x += cosf(enemy_pool[i].angle) * enemy_pool[i].speed;
        enemy_pool[i].y += sinf(enemy_pool[i].angle) * enemy_pool[i].speed;
        enemy_pool[i].frame_count++;


        enemy_pool[i].fire_timer--;
        if (enemy_pool[i].fire_timer <= 0) {
            int ptype = enemy_pool[i].type % NUM_PATTERNS;

            if (enemy_pool[i].phase == 1) {
                // fase agresiva: usar tabla de fase 2
                pattern_table_phase2[ptype](i);
            }
            else {
                pattern_table[ptype](i);
            }

            int base = 60 + diff_fire_rate_bonus();
            // en fase 2 dispara mas rapido
            if (enemy_pool[i].phase == 1) base = (int)(base * 0.6f);
            if (base < 12) base = 12;
            enemy_pool[i].fire_timer = base;
        }

        // eliminar si sale de pantalla
        if (enemy_pool[i].y > SCREEN_H + 50 ||
            enemy_pool[i].y < -200 ||
            enemy_pool[i].x < -200 ||
            enemy_pool[i].x > SCREEN_W + 200) {
            enemy_pool[i].active = 0;
        }
    }
}

// ================================
// DIBUJAR ENEMIGOS
// Jefes se dibujan mas grandes
// En fase 2 cambian de color
// ================================
void draw_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemy_pool[i].active) continue;

        float x = enemy_pool[i].x;
        float y = enemy_pool[i].y;
        int   is_boss = (enemy_pool[i].hp > 15);
        int   phase2 = (enemy_pool[i].phase == 1);

        if (is_boss) {
            // color: naranja en fase 1, magenta en fase 2
            ALLEGRO_COLOR boss_col = phase2
                ? al_map_rgb(220, 50, 220)
                : al_map_rgb(255, 80, 30);
            ALLEGRO_COLOR boss_outline = phase2
                ? al_map_rgb(255, 180, 255)
                : al_map_rgb(255, 200, 100);

            al_draw_filled_triangle(x, y - 25,
                x - 20, y + 18,
                x + 20, y + 18,
                boss_col);
            al_draw_triangle(x, y - 25,
                x - 20, y + 18,
                x + 20, y + 18,
                boss_outline, 1.5f);

            // indicador de fase 2
            if (phase2) {
                al_draw_filled_circle(x, y, 5, al_map_rgb(255, 255, 0));
            }
        }
        else {
            al_draw_filled_triangle(x, y - 15,
                x - 12, y + 10,
                x + 12, y + 10,
                al_map_rgb(255, 50, 50));
        }

        // barra de vida
        float max_hp = is_boss ? (float)enemy_pool[i].hp : 5.0f;
        float hp_ratio = (float)enemy_pool[i].hp / max_hp;
        if (hp_ratio > 1) hp_ratio = 1;
        float bar_w = is_boss ? 40.0f : 30.0f;
        float bar_top = y - (is_boss ? 32.0f : 22.0f);

        al_draw_filled_rectangle(
            x - bar_w / 2, bar_top,
            x - bar_w / 2 + bar_w * hp_ratio,
            bar_top + 4,
            is_boss
            ? (phase2 ? al_map_rgb(220, 50, 220) : al_map_rgb(255, 150, 0))
            : al_map_rgb(255, 0, 0));
    }
}