#include "weapons.h"
#include "bullet.h"
#include "enemy.h"
#include "player.h"
#include "game.h"
#include <math.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

// ================================
// VARIABLE GLOBAL
// ================================
SecondaryState secondary;
extern ALLEGRO_FONT* font;

// ================================
// NOMBRE DE CADA ARMA
// ================================
const char* secondary_name(SecondaryWeapon type) {
    switch (type) {
    case WEAPON_BOTS:         return "Bots";
    case WEAPON_VELOCITY:     return "Velocity Cannon";
    case WEAPON_SPREAD_LASER: return "Spread Laser";
    case WEAPON_MISSILE:      return "Missile";
    case WEAPON_IMPLOSION:    return "Implosion";
    case WEAPON_PLASMA:       return "Plasma Stream";
    default:                  return "Desconocida";
    }
}

// ================================
// INICIALIZAR ARMA SECUNDARIA
// ================================
void init_secondary(SecondaryWeapon type) {
    secondary.type = type;
    secondary.active = 1;
    secondary.cooldown = 0;
    secondary.implosion_radius = 0;
    secondary.implosion_timer = 0;

    switch (type) {
    case WEAPON_BOTS:
        secondary.ammo = -1;  // infinita — los bots disparan solos
        // posicionar 4 drones alrededor del jugador
        for (int i = 0; i < 4; i++) {
            secondary.bot_active[i] = 1;
            float ang = (3.14159f / 2.0f) * i;
            secondary.bot_x[i] = player.x + cosf(ang) * 40.0f;
            secondary.bot_y[i] = player.y + sinf(ang) * 40.0f;
        }
        break;
    case WEAPON_VELOCITY:     secondary.ammo = 30; break;
    case WEAPON_SPREAD_LASER: secondary.ammo = -1; break;  // continuo mientras presiona
    case WEAPON_MISSILE:      secondary.ammo = 10; break;
    case WEAPON_IMPLOSION:    secondary.ammo = 3;  break;
    case WEAPON_PLASMA:       secondary.ammo = -1; break;
    }
}

// ================================
// HELPER: enemigo más cercano al jugador
// ================================
static int nearest_enemy() {
    int   best_idx = -1;
    float best_dist = 99999.0f;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemy_pool[i].active) continue;
        float dx = enemy_pool[i].x - player.x;
        float dy = enemy_pool[i].y - player.y;
        float d = sqrtf(dx * dx + dy * dy);
        if (d < best_dist) {
            best_dist = d;
            best_idx = i;
        }
    }
    return best_idx;
}

// ================================
// DISPARO SECUNDARIO
// Llamado cuando el jugador presiona X
// ================================
void fire_secondary() {
    if (!secondary.active)   return;
    if (secondary.cooldown > 0) return;
    if (secondary.ammo == 0) return;  // sin municion

    float up = -3.14159f / 2.0f;

    switch (secondary.type) {

        // ------------------------------------
        // BOTS: los drones disparan automaticamente
        // ------------------------------------
    case WEAPON_BOTS:
        break;

        // ------------------------------------
        // VELOCITY CANNON: 1 rayo muy rapido
        // en linea recta, alta potencia (mas daño)
        // implementado como 3 balas superpuestas
        // ------------------------------------
    case WEAPON_VELOCITY:
        fire_single(player.x - 1, player.y, up, 14.0f, 0);
        fire_single(player.x, player.y, up, 14.0f, 0);
        fire_single(player.x + 1, player.y, up, 14.0f, 0);
        secondary.cooldown = 8;
        if (secondary.ammo > 0) secondary.ammo--;
        break;

        // ------------------------------------
        // SPREAD LASER: abanico amplio de 7 balas
        // cubre los costados de la pantalla
        // ------------------------------------
    case WEAPON_SPREAD_LASER:
    {
        float spread = 0.35f;
        for (int i = -3; i <= 3; i++) {
            fire_single(player.x, player.y,
                up + spread * i, 7.0f, 0);
        }
        secondary.cooldown = 4;
    }
    break;

    // ------------------------------------
    // MISSILE: misil que apunta al enemigo
    // más cercano automaticamente
    // ------------------------------------
    case WEAPON_MISSILE:
    {
        int target = nearest_enemy();
        if (target >= 0) {
            float tx = enemy_pool[target].x;
            float ty = enemy_pool[target].y;
            float angle = atan2f(ty - player.y, tx - player.x);
            fire_single(player.x, player.y, angle, 6.0f, 0);
        }
        else {
            // sin objetivo — dispara recto
            fire_single(player.x, player.y, up, 6.0f, 0);
        }
        secondary.cooldown = 20;
        if (secondary.ammo > 0) secondary.ammo--;
    }
    break;

    // ------------------------------------
    // IMPLOSION: rayo de gravedad
    // atrae todas las balas enemigas al centro
    // y las destruye; aplasta enemigos cercanos
    // ------------------------------------
    case WEAPON_IMPLOSION:
    {
        float radius = 120.0f;

        // destruir balas enemigas en rango
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullet_pool[i].active) continue;
            if (bullet_pool[i].owner != 1) continue;
            float dx = bullet_pool[i].x - player.x;
            float dy = bullet_pool[i].y - player.y;
            if (sqrtf(dx * dx + dy * dy) <= radius) {
                free_bullet(i);
                player.score += 2;
            }
        }

        // dañar enemigos cercanos
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemy_pool[i].active) continue;
            float dx = enemy_pool[i].x - player.x;
            float dy = enemy_pool[i].y - player.y;
            if (sqrtf(dx * dx + dy * dy) <= radius) {
                enemy_pool[i].hp -= 3;
                player.shots_hit++;
                if (enemy_pool[i].hp <= 0) enemy_die(i);
            }
        }

        secondary.implosion_radius = radius;
        secondary.implosion_timer = 20;   // frames de efecto visual
        secondary.cooldown = 90;
        if (secondary.ammo > 0) secondary.ammo--;
    }
    break;

    // ------------------------------------
    // PLASMA STREAM: rafaga rapida de plasma
    // 5 balas en rapida sucesion al frente
    // ------------------------------------
    case WEAPON_PLASMA:
        for (int i = 0; i < 3; i++) {
            fire_single(player.x + (i - 1) * 6, player.y,
                up, 10.0f + i * 0.5f, 0);
        }
        secondary.cooldown = 3;
        break;
    }
}

// ================================
// UPDATE SECUNDARIO
// Llamado una vez por frame
// ================================
void update_secondary() {
    if (!secondary.active) return;

    if (secondary.cooldown > 0) secondary.cooldown--;

    switch (secondary.type) {

        // ------------------------------------
        // BOTS: orbitan al jugador y disparan
        // automaticamente al enemigo mas cercano
        // ------------------------------------
    case WEAPON_BOTS:
    {
        static int bot_fire_timer = 0;
        bot_fire_timer++;

        for (int i = 0; i < 4; i++) {
            if (!secondary.bot_active[i]) continue;

            // orbitar alrededor del jugador
            float target_ang = (3.14159f / 2.0f) * i +
                (float)bot_fire_timer * 0.02f;
            float target_x = player.x + cosf(target_ang) * 40.0f;
            float target_y = player.y + sinf(target_ang) * 40.0f;

            // suavizar movimiento
            secondary.bot_x[i] += (target_x - secondary.bot_x[i]) * 0.2f;
            secondary.bot_y[i] += (target_y - secondary.bot_y[i]) * 0.2f;

            // disparar al enemigo mas cercano desde la posicion del bot
            if (bot_fire_timer % 20 == i * 5) {
                int tgt = nearest_enemy();
                if (tgt >= 0) {
                    float angle = atan2f(
                        enemy_pool[tgt].y - secondary.bot_y[i],
                        enemy_pool[tgt].x - secondary.bot_x[i]);
                    fire_single(secondary.bot_x[i], secondary.bot_y[i],
                        angle, 7.0f, 0);
                    player.shots_fired++;
                }
            }
        }
    }
    break;

    // ------------------------------------
    // IMPLOSION: contar el efecto visual
    // ------------------------------------
    case WEAPON_IMPLOSION:
        if (secondary.implosion_timer > 0) {
            secondary.implosion_timer--;
            secondary.implosion_radius *= 1.1f;  // onda expansiva
        }
        else {
            secondary.implosion_radius = 0;
        }
        break;

    default:
        break;
    }
}

// ================================
// DIBUJAR ARMA SECUNDARIA
// ================================
void draw_secondary() {
    if (!secondary.active) return;

    switch (secondary.type) {

        // bots: pequeños triángulos cian
    case WEAPON_BOTS:
        for (int i = 0; i < 4; i++) {
            if (!secondary.bot_active[i]) continue;
            float bx = secondary.bot_x[i];
            float by = secondary.bot_y[i];
            al_draw_filled_triangle(
                bx, by - 8,
                bx - 6, by + 5,
                bx + 6, by + 5,
                al_map_rgb(0, 220, 220));
        }
        break;

    case WEAPON_IMPLOSION:
        if (secondary.implosion_timer > 0) {
            float alpha = (float)secondary.implosion_timer / 20.0f;
            al_draw_circle(player.x, player.y,
                secondary.implosion_radius,
                al_map_rgba(180, 100, 255, (int)(alpha * 200)),
                2.0f + (1.0f - alpha) * 3.0f);
        }
        break;

    default:
        break;
    }

    // HUD del arma secundaria (esquina inferior derecha)
    al_draw_textf(font, al_map_rgb(180, 255, 180),
        SCREEN_W - 10, SCREEN_H - 50, ALLEGRO_ALIGN_RIGHT,
        "2nd: %s", secondary_name(secondary.type));

    if (secondary.ammo >= 0) {
        al_draw_textf(font, al_map_rgb(150, 200, 150),
            SCREEN_W - 10, SCREEN_H - 35, ALLEGRO_ALIGN_RIGHT,
            "Ammo: %d", secondary.ammo);
    }
    else {
        al_draw_text(font, al_map_rgb(100, 180, 100),
            SCREEN_W - 10, SCREEN_H - 35, ALLEGRO_ALIGN_RIGHT,
            "Ammo: inf");
    }
}

// ================================
// RECOGER ARMA NUEVA
// ================================
void pick_up_secondary(SecondaryWeapon type) {
    init_secondary(type);
}