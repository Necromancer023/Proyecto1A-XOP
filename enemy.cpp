#include "enemy.h"
#include "bullet.h"
#include "player.h"
#include "item.h"
#include "game.h"
#include <math.h>
#include <allegro5/allegro_primitives.h>

// ================================
// VARIABLE GLOBAL
// ================================
Enemy enemy_pool[MAX_ENEMIES];

// ================================
// TABLA DE PATRONES DE DISPARO
// ================================
typedef void (*FirePattern)(int idx);

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

FirePattern pattern_table[] = {
    pattern_aimed,
    pattern_circle,
    pattern_spread,
    pattern_double_circle,
    pattern_spiral
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
        float offset_x = (t == 0) ? 0 : 30.0f;  // ligeramente separados

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemy_pool[i].active) continue;

            // aplicar bonus de dificultad
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
// ================================
void enemy_die(int idx) {
    enemy_pool[idx].active = 0;
    player.score += 100 * (1 + player.combo / 10);
    player.combo++;

    spawn_dal(enemy_pool[idx].x, enemy_pool[idx].y);
    spawn_dal(enemy_pool[idx].x - 15, enemy_pool[idx].y);
    spawn_dal(enemy_pool[idx].x + 15, enemy_pool[idx].y);
}

// ================================
// ACTUALIZAR ENEMIGOS
// ================================
void update_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemy_pool[i].active) continue;

        enemy_pool[i].x += cos(enemy_pool[i].angle) * enemy_pool[i].speed;
        enemy_pool[i].y += sin(enemy_pool[i].angle) * enemy_pool[i].speed;
        enemy_pool[i].frame_count++;

        enemy_pool[i].fire_timer--;
        if (enemy_pool[i].fire_timer <= 0) {
            if (enemy_pool[i].type < NUM_PATTERNS) {
                pattern_table[enemy_pool[i].type](i);
            }
            // resetear con factor de dificultad
            int base = 60 + diff_fire_rate_bonus();
            if (base < 15) base = 15;
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
// Los jefes se dibujan mas grandes
// ================================
void draw_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemy_pool[i].active) continue;

        float x = enemy_pool[i].x;
        float y = enemy_pool[i].y;

        // jefes (hp > 15) se dibujan mas grandes y de color diferente
        int is_boss = (enemy_pool[i].hp > 15);

        if (is_boss) {
            // jefe: hexagono rojo mas grande
            al_draw_filled_triangle(
                x, y - 25,
                x - 20, y + 18,
                x + 20, y + 18,
                al_map_rgb(255, 80, 30));
            al_draw_triangle(
                x, y - 25,
                x - 20, y + 18,
                x + 20, y + 18,
                al_map_rgb(255, 200, 100), 1.5f);
        }
        else {
            // enemigo normal
            al_draw_filled_triangle(
                x, y - 15,
                x - 12, y + 10,
                x + 12, y + 10,
                al_map_rgb(255, 50, 50));
        }

        // barra de vida
        float max_hp = is_boss ? (float)enemy_pool[i].hp : 5.0f;
        float hp_ratio = (float)enemy_pool[i].hp / max_hp;
        if (hp_ratio > 1) hp_ratio = 1;
        float bar_w = is_boss ? 40.0f : 30.0f;

        al_draw_filled_rectangle(
            x - bar_w / 2, y - (is_boss ? 32 : 22),
            x - bar_w / 2 + bar_w * hp_ratio,
            y - (is_boss ? 28 : 18),
            is_boss ? al_map_rgb(255, 150, 0) : al_map_rgb(255, 0, 0));
    }
}