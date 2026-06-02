#include "player.h"
#include "bullet.h"
#include "weapons.h"
#include "game.h"
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

// ================================
// VARIABLE GLOBAL
// ================================
Player player;

extern GameState game_state;
extern int       difficulty;

// teclas — se actualiza cada frame
static ALLEGRO_KEYBOARD_STATE kb;

// cooldowns
static int shoot_timer = 0;
static int weapon_timer = 0;
static int sec_wep_timer = 0;  // timer para cambio de arma secundaria

// invulnerabilidad tras recibir daño (en frames)
static int invincible_timer = 0;
#define INVINCIBLE_FRAMES 90

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
    invincible_timer = 0;
    shoot_timer = 0;
    weapon_timer = 0;
    sec_wep_timer = 0;
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
    if (player.x < 10)             player.x = 10;
    if (player.x > SCREEN_W - 10)  player.x = SCREEN_W - 10;
    if (player.y < 10)             player.y = 10;
    if (player.y > SCREEN_H - 10)  player.y = SCREEN_H - 10;

    // --- disparo primario (Z) ---
    if (shoot_timer > 0) shoot_timer--;
    if (al_key_down(&kb, ALLEGRO_KEY_Z) && shoot_timer == 0) {
        player_shoot();
        shoot_timer = 6;
    }

    // --- disparo secundario (X) ---
    if (al_key_down(&kb, ALLEGRO_KEY_X)) {
        fire_secondary();
    }

    // --- reflect shield / bomba (A) ---
    // (X era el anterior; A es la nueva tecla para bomba/reflect)
    // mantenemos X para arma secundaria y A para reflect
    // pero si no hay arma secundaria, X tambien puede reflect

    // --- cambio de arma primaria (C) ---
    if (weapon_timer > 0) weapon_timer--;
    if (al_key_down(&kb, ALLEGRO_KEY_C) && weapon_timer == 0) {
        player_change_weapon();
        weapon_timer = 15;
    }

    // --- cambio de arma secundaria (V) ---
    if (sec_wep_timer > 0) sec_wep_timer--;
    if (al_key_down(&kb, ALLEGRO_KEY_V) && sec_wep_timer == 0) {
        int next = ((int)secondary.type + 1) % 6;
        pick_up_secondary((SecondaryWeapon)next);
        sec_wep_timer = 20;
    }

    // --- reflect shield (A) ---
    if (al_key_down(&kb, ALLEGRO_KEY_A)) {
        player_reflect_shield();
    }

    // --- invulnerabilidad temporal ---
    if (invincible_timer > 0) invincible_timer--;
}

// ================================
// DISPARO SEGUN ARMA ACTUAL
// ================================
void player_shoot() {
    player.shots_fired++;

    float up = -3.14159f / 2.0f;

    switch (player.weapon) {
    case 1:
        fire_single(player.x, player.y, up, 8.0f, 0);
        break;
    case 2:
        fire_single(player.x - 10, player.y, up, 8.0f, 0);
        fire_single(player.x + 10, player.y, up, 8.0f, 0);
        break;
    case 3:
        fire_single(player.x, player.y, up, 8.0f, 0);
        fire_single(player.x, player.y, up - 0.2f, 7.0f, 0);
        fire_single(player.x, player.y, up + 0.2f, 7.0f, 0);
        break;
    case 4:
        fire_single(player.x - 15, player.y, up, 8.0f, 0);
        fire_single(player.x - 5, player.y, up, 8.0f, 0);
        fire_single(player.x + 5, player.y, up, 8.0f, 0);
        fire_single(player.x + 15, player.y, up, 8.0f, 0);
        break;
    case 5:
        fire_single(player.x, player.y, up, 9.0f, 0);
        fire_single(player.x, player.y, up - 0.4f, 7.0f, 0);
        fire_single(player.x, player.y, up + 0.4f, 7.0f, 0);
        break;
    case 6:
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

    float radio = 80.0f;

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullet_pool[i].active) continue;
        if (bullet_pool[i].owner != 1) continue;

        float dx = bullet_pool[i].x - player.x;
        float dy = bullet_pool[i].y - player.y;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist <= radio) {
            bullet_pool[i].angle += 3.14159f;
            bullet_pool[i].owner = 0;
            player.shots_fired++;
        }
    }
}

// ================================
// RECIBIR DAÑO — GAME OVER REAL
// ================================
void player_take_damage() {
    if (invincible_timer > 0) return;  // frames de invulnerabilidad

    player.hp--;
    player.combo = 0;
    invincible_timer = INVINCIBLE_FRAMES;  // 1.5 segundos de invulnerabilidad

    if (player.hp <= 0) {
        // GAME OVER real — cambia el estado del juego
        game_state = STATE_GAME_OVER;
    }
}

// ================================
// CAMBIAR ARMA PRIMARIA
// ================================
void player_change_weapon() {
    player.weapon++;
    if (player.weapon > MAX_WEAPONS)
        player.weapon = 1;
}

// ================================
// DIBUJAR JUGADOR
// Parpadea cuando esta invulnerable
// ================================
void draw_player() {
    // parpadear cuando es invulnerable
    if (invincible_timer > 0 && (invincible_timer / 5) % 2 == 0)
        return;

    float x = player.x;
    float y = player.y;

    al_draw_filled_triangle(
        x, y - 15,
        x - 12, y + 10,
        x + 12, y + 10,
        al_map_rgb(0, 200, 255));

    // hitbox de debug
    al_draw_circle(x, y, 4, al_map_rgb(255, 255, 0), 1.0f);
}