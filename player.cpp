#include "player.h"
#include "bullet.h"
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

// ================================
// VARIABLE GLOBAL
// ================================
Player player;

// teclas — se actualiza cada frame
static ALLEGRO_KEYBOARD_STATE kb;

// cooldown de disparo en frames
static int shoot_timer = 0;
static int weapon_timer = 0;

// ================================
// INICIALIZAR JUGADOR
// ================================
void init_player() {
    player.x = SCREEN_W / 2.0f;
    player.y = SCREEN_H - 80.0f;
    player.speed = PLAYER_SPEED;
    player.hp = PLAYER_HP;
    player.bombs = PLAYER_BOMBS;
    player.weapon = 1;
    player.score = 0;
    player.combo = 0;
    player.shots_fired = 0;
    player.shots_hit = 0;
    player.shots_missed = 0;
}

// ================================
// ACTUALIZAR JUGADOR
// ================================
void update_player() {
    al_get_keyboard_state(&kb);

    // --- movimiento ---
    if (al_key_down(&kb, ALLEGRO_KEY_LEFT))
        player.x -= player.speed;
    if (al_key_down(&kb, ALLEGRO_KEY_RIGHT))
        player.x += player.speed;
    if (al_key_down(&kb, ALLEGRO_KEY_UP))
        player.y -= player.speed;
    if (al_key_down(&kb, ALLEGRO_KEY_DOWN))
        player.y += player.speed;

    // --- limites de pantalla ---
    if (player.x < 10)           player.x = 10;
    if (player.x > SCREEN_W - 10) player.x = SCREEN_W - 10;
    if (player.y < 10)           player.y = 10;
    if (player.y > SCREEN_H - 10) player.y = SCREEN_H - 10;

    // --- disparo ---
    if (shoot_timer > 0) shoot_timer--;

    if (al_key_down(&kb, ALLEGRO_KEY_Z) && shoot_timer == 0) {
        player_shoot();
        shoot_timer = 6;    // dispara cada 6 frames (10 veces por segundo)
    }

    // --- cambio de arma ---
    if (weapon_timer > 0) weapon_timer--;

    if (al_key_down(&kb, ALLEGRO_KEY_C) && weapon_timer == 0) {
        player_change_weapon();
        weapon_timer = 15;  // evita cambiar demasiado rapido
    }

    // --- reflect shield ---
    if (al_key_down(&kb, ALLEGRO_KEY_X)) {
        player_reflect_shield();
    }
}

// ================================
// DISPARO SEGUN ARMA ACTUAL
// ================================
void player_shoot() {
    player.shots_fired++;

    float up = -3.14159f / 2.0f;   // angulo hacia arriba

    switch (player.weapon) {
    case 1:
        // disparo simple al frente
        fire_single(player.x, player.y, up, 8.0f, 0);
        break;
    case 2:
        // doble disparo
        fire_single(player.x - 10, player.y, up, 8.0f, 0);
        fire_single(player.x + 10, player.y, up, 8.0f, 0);
        break;
    case 3:
        // triple disparo en abanico
        fire_single(player.x, player.y, up, 8.0f, 0);
        fire_single(player.x, player.y, up - 0.2f, 7.0f, 0);
        fire_single(player.x, player.y, up + 0.2f, 7.0f, 0);
        break;
    case 4:
        // cuadruple
        fire_single(player.x - 15, player.y, up, 8.0f, 0);
        fire_single(player.x - 5, player.y, up, 8.0f, 0);
        fire_single(player.x + 5, player.y, up, 8.0f, 0);
        fire_single(player.x + 15, player.y, up, 8.0f, 0);
        break;
    case 5:
        // disparo diagonal + frente
        fire_single(player.x, player.y, up, 9.0f, 0);
        fire_single(player.x, player.y, up - 0.4f, 7.0f, 0);
        fire_single(player.x, player.y, up + 0.4f, 7.0f, 0);
        break;
    case 6:
        // maximo spread
        fire_single(player.x, player.y, up, 9.0f, 0);
        fire_single(player.x, player.y, up - 0.3f, 8.0f, 0);
        fire_single(player.x, player.y, up + 0.3f, 8.0f, 0);
        fire_single(player.x, player.y, up - 0.6f, 7.0f, 0);
        fire_single(player.x, player.y, up + 0.6f, 7.0f, 0);
        break;
    }
}

// ================================
// REFLECT SHIELD
// ================================
void player_reflect_shield() {
    if (player.bombs <= 0) return;
    player.bombs--;

    float radio = 80.0f;    // radio de efecto

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullet_pool[i].active) continue;
        if (bullet_pool[i].owner != 1) continue;    // solo balas enemigas

        float dx = bullet_pool[i].x - player.x;
        float dy = bullet_pool[i].y - player.y;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist <= radio) {
            // invertir direccion de la bala
            bullet_pool[i].angle += 3.14159f;
            bullet_pool[i].owner = 0;  // ahora es del jugador
            player.shots_fired++;
        }
    }
}

// ================================
// RECIBIR DAÑO
// ================================
void player_take_damage() {
    player.hp--;
    player.combo = 0;   // perder combo al recibir daño

    if (player.hp <= 0) {
        // game over — por ahora solo reinicia posicion
        // tu compañero conectara esto con el sistema de estados
        init_player();
    }
}

// ================================
// CAMBIAR ARMA
// ================================
void player_change_weapon() {
    player.weapon++;
    if (player.weapon > MAX_WEAPONS)
        player.weapon = 1;
}

// ================================
// DIBUJAR JUGADOR
// ================================
void draw_player() {
    // por ahora un triangulo azul hasta tener sprites
    float x = player.x;
    float y = player.y;

    al_draw_filled_triangle(
        x, y - 15,    // punta arriba
        x - 12, y + 10,    // esquina izquierda
        x + 12, y + 10,    // esquina derecha
        al_map_rgb(0, 200, 255)
    );

    // hitbox de debug (circulo pequeño en el centro)
    al_draw_circle(x, y, 4, al_map_rgb(255, 255, 0), 1.0f);
}