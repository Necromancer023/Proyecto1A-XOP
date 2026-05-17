#include "bullet.h"
#include <math.h>
#include <allegro5/allegro_primitives.h>

// ================================
// VARIABLES GLOBALES
// ================================
Bullet bullet_pool[MAX_BULLETS];
int free_bullets[MAX_BULLETS];
int free_top = 0;

// ================================
// INICIALIZAR POOL
// ================================
void init_bullet_pool() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullet_pool[i].active = 0;
        free_bullets[free_top++] = i;
    }
}

// ================================
// SACAR BALA DEL POOL
// ================================
int alloc_bullet() {
    if (free_top == 0) return -1;   // pool lleno
    return free_bullets[--free_top];
}

// ================================
// DEVOLVER BALA AL POOL
// ================================
void free_bullet(int idx) {
    bullet_pool[idx].active = 0;
    free_bullets[free_top++] = idx;
}

// ================================
// ACTUALIZAR TODAS LAS BALAS
// ================================
void update_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullet_pool[i].active) continue;

        // mover con coordenadas polares (estilo Touhou)
        bullet_pool[i].x += cos(bullet_pool[i].angle) * bullet_pool[i].speed;
        bullet_pool[i].y += sin(bullet_pool[i].angle) * bullet_pool[i].speed;

        // eliminar si sale de pantalla
        if (bullet_pool[i].x < 0 || bullet_pool[i].x > SCREEN_W ||
            bullet_pool[i].y < 0 || bullet_pool[i].y > SCREEN_H) {
            free_bullet(i);
        }
    }
}

// ================================
// DIBUJAR TODAS LAS BALAS
// ================================
void draw_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullet_pool[i].active) continue;

        // jugador = azul, enemigo = rojo
        ALLEGRO_COLOR color = (bullet_pool[i].owner == 0)
            ? al_map_rgb(0, 150, 255)
            : al_map_rgb(255, 50, 50);

        al_draw_filled_circle(bullet_pool[i].x, bullet_pool[i].y, 4, color);
    }
}

// ================================
// PATRONES DE DISPARO
// ================================

// disparo simple en un angulo dado
void fire_single(float x, float y, float angle, float speed, int owner) {
    int idx = alloc_bullet();
    if (idx == -1) return;

    bullet_pool[idx].x = x;
    bullet_pool[idx].y = y;
    bullet_pool[idx].angle = angle;
    bullet_pool[idx].speed = speed;
    bullet_pool[idx].owner = owner;
    bullet_pool[idx].active = 1;
}

// circulo completo de balas
void fire_circle(float x, float y, int cantidad, float speed) {
    for (int i = 0; i < cantidad; i++) {
        float angle = (2.0f * 3.14159f / cantidad) * i;
        fire_single(x, y, angle, speed, 1);
    }
}

// bala apuntada directamente al jugador
void fire_aimed(float ex, float ey, float px, float py, float speed) {
    float angle = atan2(py - ey, px - ex);
    fire_single(ex, ey, angle, speed, 1);
}

// abanico de balas apuntando al jugador
void fire_spread(float ex, float ey, float px, float py, float speed, int cantidad) {
    float base = atan2(py - ey, px - ex);
    float separacion = 0.2f;   // radianes entre cada bala
    float inicio = base - (separacion * (cantidad - 1)) / 2.0f;

    for (int i = 0; i < cantidad; i++) {
        fire_single(ex, ey, inicio + separacion * i, speed, 1);
    }
}