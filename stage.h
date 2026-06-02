#pragma once
#ifndef STAGE_H
#define STAGE_H

#include "game.h"

// ================================
// CONSTANTES DE NIVELES
// ================================
#define MAX_STAGES       4
#define MAX_WAVES        8   // oleadas por nivel
#define BOSS_HP_BASE     30

// ================================
// DESCRIPTOR DE UNA OLEADA
// define cuántos enemigos, de qué
// tipo, en qué posiciones y con
// qué cadencia spawnean
// ================================
typedef struct {
    int   enemy_type;       // 0-4 
    int   count;            // cantidad de enemigos en esta oleada
    float speed;            // velocidad base
    int   hp;               // vida de cada enemigo
    float spawn_interval;   // frames entre cada spawn del grupo
    int   formation;        // 0=línea, 1=V, 2=diamante, 3=aleatorio
} WaveDescriptor;

// ================================
// DESCRIPTOR DE UN NIVEL
// ================================
typedef struct {
    const char* name;           // nombre del nivel
    int            wave_count;     // cuántas oleadas tiene
    WaveDescriptor waves[MAX_WAVES];
    int            has_boss;       // 1 si termina en jefe
    int            boss_type;      // tipo del jefe 
    int            boss_hp;        // vida del jefe
    float          bg_scroll_speed;// velocidad de scroll del fondo
} StageDescriptor;

// ================================
// ESTADO DEL STAGE ACTUAL
// ================================
typedef struct {
    int   current_stage;    // 0-3
    int   current_wave;     // 0..wave_count-1
    int   second_loop;      // 1 = segunda vuelta (patrones más difíciles)
    int   wave_timer;       // frames hasta la siguiente oleada
    int   spawned_in_wave;  // enemigos ya spawnados en esta oleada
    int   spawn_timer;      // frames entre spawns individuales
    int   boss_active;      // 1 si el jefe está vivo
    int   boss_idx;         // índice en enemy_pool del jefe actual
    int   stage_clear;      // 1 cuando todos los enemigos murieron
    int   transition_timer; // frames de pantalla de transición
} StageState;

// ================================
// VARIABLE GLOBAL
// ================================
extern StageState stage_state;
extern StageDescriptor stages[MAX_STAGES];

// ================================
// FUNCIONES
// ================================
void init_stages();
void init_stage_state();
void update_stage();
void next_wave();
void next_stage();
void spawn_wave_formation(WaveDescriptor* wave);
void spawn_boss(int stage_idx);
int  count_active_enemies();
int  is_stage_clear();
void draw_stage_hud();         // nombre del nivel y barra de boss

#endif