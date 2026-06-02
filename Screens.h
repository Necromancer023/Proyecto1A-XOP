#pragma once
#ifndef SCREENS_H
#define SCREENS_H

#include "game.h"

// ================================
// PANTALLA DE MENU PRINCIPAL
// ================================
void draw_menu();
void update_menu(ALLEGRO_EVENT* ev);

// ================================
// PANTALLA DE OPCIONES
// ================================
void draw_options();
void update_options(ALLEGRO_EVENT* ev);

// ================================
// PANTALLA DE GAME OVER
// ================================
void draw_game_over();
void update_game_over(ALLEGRO_EVENT* ev);

// ================================
// PANTALLA DE VICTORIA
// ================================
void draw_victory();
void update_victory(ALLEGRO_EVENT* ev);

// ================================
// PANTALLA DE ESTADISTICAS / RANKING
// ================================
void draw_stats_screen();
void update_stats_screen(ALLEGRO_EVENT* ev);

// ================================
// PANTALLA NAME ENTRY
// Se muestra al terminar una partida
// para ingresar el nombre del jugador
// ================================
void draw_name_entry();
void update_name_entry(ALLEGRO_EVENT* ev);

// ================================
// HELPERS
// ================================
void screens_init();            // limpiar buffers de input
extern char name_entry_buffer[50];
extern int  name_entry_len;
extern int  name_entry_done;    // 1 cuando el jugador presiono Enter

#endif
