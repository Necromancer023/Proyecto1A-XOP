#include "collision.h"
#include "bullet.h"
#include "enemy.h"
#include "player.h"
#include <math.h>

// ================================
// DETECCION DE COLISION CIRCULAR
// sin sqrt para mayor rendimiento
// ================================
int circles_collide(float x1, float y1, float r1,
    float x2, float y2, float r2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    float dist2 = dx * dx + dy * dy;
    float rsum = r1 + r2;
    return dist2 <= rsum * rsum;
}

// ================================
// BALAS ENEMIGAS vs JUGADOR
// ================================
void check_player_hit() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullet_pool[i].active)   continue;
        if (bullet_pool[i].owner != 1) continue;  // solo balas enemigas

        if (circles_collide(
            player.x, player.y, 4.0f,              // hitbox jugador
            bullet_pool[i].x, bullet_pool[i].y, 3.0f)) {  // hitbox bala

            free_bullet(i);
            player_take_damage();
        }
    }
}

// ================================
// BALAS DEL JUGADOR vs ENEMIGOS
// ================================
void check_enemy_hit() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullet_pool[i].active)   continue;
        if (bullet_pool[i].owner != 0) continue;  // solo balas del jugador

        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!enemy_pool[j].active) continue;

            if (circles_collide(
                bullet_pool[i].x, bullet_pool[i].y, 3.0f,   // hitbox bala
                enemy_pool[j].x, enemy_pool[j].y, 15.0f)) { // hitbox enemigo

                free_bullet(i);
                enemy_pool[j].hp--;
                player.shots_hit++;

                if (enemy_pool[j].hp <= 0) {
                    enemy_die(j);
                }

                break;  // una bala solo golpea un enemigo
            }
        }
    }
}