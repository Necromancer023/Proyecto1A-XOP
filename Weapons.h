#pragma once
#ifndef WEAPONS_H
#define WEAPONS_H

#include "game.h"

// ================================
// ARMAS SECUNDARIAS (6 tipos)
// Se alternan con la tecla C
// ================================
typedef enum {
    WEAPON_BOTS = 0,  // 4 drones que siguen la nave
    WEAPON_VELOCITY = 1,  // rayo en línea recta de alta potencia
    WEAPON_SPREAD_LASER = 2,  // disparo en arco a los lados
    WEAPON_MISSILE = 3,  // misil que sigue al enemigo mas cercano
    WEAPON_IMPLOSION = 4,  // rayo de gravedad — atrae y aplasta enemigos
    WEAPON_PLASMA = 5   // rafaga rapida de plasma
} SecondaryWeapon;

// ================================
// ESTADO DEL ARMA SECUNDARIA
// ================================
typedef struct {
    SecondaryWeapon type;
    int  ammo;          // municion restante 
    int  cooldown;      // frames hasta poder disparar de nuevo
    int  active;        // 1 si el arma esta equipada

    // para BOTS: posicion de los 4 drones
    float bot_x[4];
    float bot_y[4];
    int   bot_active[4];

    // para IMPLOSION: radio y duracion del efecto
    float implosion_radius;
    int   implosion_timer;
} SecondaryState;

// ================================
// VARIABLE GLOBAL
// ================================
extern SecondaryState secondary;

// ================================
// FUNCIONES
// ================================
void init_secondary(SecondaryWeapon type);
void update_secondary();
void draw_secondary();
void fire_secondary();
void pick_up_secondary(SecondaryWeapon type);

// nombre legible del arma
const char* secondary_name(SecondaryWeapon type);

#endif
