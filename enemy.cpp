#include "enemy.h"
#include "bullet.h"
#include "player.h"
#include <math.h>
#include <allegro5/allegro_primitives.h>

// ================================
// VARIABLE GLOBAL
// ================================
Enemy enemy_pool[MAX_ENEMIES];

// ================================
// TYPEDEF PARA LA TABLA DE PATRONES
// un puntero a funcion que recibe
// el indice del enemigo en el pool
// ================================
typedef void (*FirePattern)(int idx);

// ================================
// PATRONES DE DISPARO
// cada funcion define el comportamiento
// de disparo de un tipo de enemigo
// ================================

// tipo 0 — dispara directo al jugador
void pattern_aimed(int idx) {
    fire_aimed(
        enemy_pool[idx].x, enemy_pool[idx].y,
        player.x, player.y,
        3.0f
    );
}

// tipo 1 — circulo de 12 balas en todas direcciones
void pattern_circle(int idx) {
    fire_circle(enemy_pool[idx].x, enemy_pool[idx].y, 12, 2.5f);
}

// tipo 2 — abanico de 5 balas apuntando al jugador
void pattern_spread(int idx) {
    fire_spread(
        enemy_pool[idx].x, enemy_pool[idx].y,
        player.x, player.y,
        3.0f, 5
    );
}

// tipo 3 — doble circulo para jefes basicos
// dispara dos anillos de balas a velocidades distintas
void pattern_double_circle(int idx) {
    fire_circle(enemy_pool[idx].x, enemy_pool[idx].y, 8, 2.0f);
    fire_circle(enemy_pool[idx].x, enemy_pool[idx].y, 8, 3.5f);
}

// tipo 4 — espiral usando frame_count del enemigo
// el angulo aumenta con el tiempo creando efecto de espiral
void pattern_spiral(int idx) {
    float angle = (enemy_pool[idx].frame_count * 0.15f);
    fire_single(enemy_pool[idx].x, enemy_pool[idx].y, angle, 3.0f, 1);
}

// ================================
// TABLA DE PATRONES
// array de punteros a funcion
// el tipo del enemigo es el indice
// permite seleccionar patron en O(1)
// sin usar switch
// ================================
FirePattern pattern_table[] = {
    pattern_aimed,          // tipo 0
    pattern_circle,         // tipo 1
    pattern_spread,         // tipo 2
    pattern_double_circle,  // tipo 3
    pattern_spiral          // tipo 4
};

// total de patrones definidos
#define NUM_PATTERNS 5

// ================================
// INICIALIZAR POOL DE ENEMIGOS
// marca todos como inactivos al inicio
// ================================
void init_enemy_pool() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemy_pool[i].active = 0;
    }
}

// ================================
// SPAWNEAR UN ENEMIGO
// busca el primer slot inactivo
// y configura sus valores iniciales
// ================================
void spawn_enemy(float x, float y, float angle, float speed, int hp, int type) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemy_pool[i].active) continue;

        enemy_pool[i].x = x;
        enemy_pool[i].y = y;
        enemy_pool[i].angle = angle;
        enemy_pool[i].speed = speed;
        enemy_pool[i].hp = hp;
        enemy_pool[i].active = 1;
        enemy_pool[i].type = type;
        enemy_pool[i].fire_timer = 60;  // primer disparo al segundo
        enemy_pool[i].phase = 0;
        enemy_pool[i].frame_count = 0;
        return;
    }
    // si llega aqui el pool esta lleno
}

// ================================
// CUANDO UN ENEMIGO MUERE
// desactiva el enemigo, suma puntaje
// y aumenta el combo del jugador
// ================================
void enemy_die(int idx) {
    enemy_pool[idx].active = 0;
    player.score += 100;
    player.combo++;

    // aqui se conecta el spawn de monedas DAL
}

// ================================
// ACTUALIZAR TODOS LOS ENEMIGOS
// se llama una vez por frame
// mueve, controla disparo y elimina
// enemigos que salen de pantalla
// ================================
void update_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemy_pool[i].active) continue;

        // mover con coordenadas polares
        enemy_pool[i].x += cos(enemy_pool[i].angle) * enemy_pool[i].speed;
        enemy_pool[i].y += sin(enemy_pool[i].angle) * enemy_pool[i].speed;
        enemy_pool[i].frame_count++;

        // contar frames y disparar cuando llega a cero
        enemy_pool[i].fire_timer--;
        if (enemy_pool[i].fire_timer <= 0) {

            // seleccionar patron en O(1) con la tabla
            if (enemy_pool[i].type < NUM_PATTERNS) {
                pattern_table[enemy_pool[i].type](i);
            }

            enemy_pool[i].fire_timer = 60;  // resetear a 60 frames
        }

        // eliminar si sale completamente de pantalla
        if (enemy_pool[i].y > SCREEN_H + 50 ||
            enemy_pool[i].y < -200 ||
            enemy_pool[i].x < -200 ||
            enemy_pool[i].x > SCREEN_W + 200) {
            enemy_pool[i].active = 0;
        }
    }
}

// ================================
// DIBUJAR TODOS LOS ENEMIGOS
// dibuja triangulo rojo por cada
// enemigo activo mas su barra de vida
// ================================
void draw_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemy_pool[i].active) continue;

        float x = enemy_pool[i].x;
        float y = enemy_pool[i].y;

        // triangulo rojo — reemplazar con sprite
        al_draw_filled_triangle(
            x, y - 15,
            x - 12, y + 10,
            x + 12, y + 10,
            al_map_rgb(255, 50, 50)
        );

        // barra de vida proporcional al hp actual
        float hp_ratio = enemy_pool[i].hp / 5.0f;
        al_draw_filled_rectangle(
            x - 15, y - 22,
            x - 15 + 30 * hp_ratio, y - 18,
            al_map_rgb(255, 0, 0)
        );
    }
}