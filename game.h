#ifndef GAME_H
#define GAME_H

#include <allegro5/allegro.h>

// ================================
// CONSTANTES
// ================================
#define SCREEN_W        480
#define SCREEN_H        640
#define FPS             60

#define MAX_BULLETS     2000
#define MAX_ENEMIES     100
#define MAX_ITEMS       200

#define PLAYER_SPEED    4.0f
#define PLAYER_HP       5
#define PLAYER_BOMBS    3
#define MAX_WEAPONS     6

// ================================
// STRUCTS
// ================================

typedef struct {
    float x, y;
    float angle;
    float speed;
    int active;
    int owner;      // 0 = jugador, 1 = enemigo
} Bullet;

typedef struct {
    float x, y;
    float speed;
    int hp;
    int bombs;
    int weapon;
    int score;
    int combo;
    int shots_fired;
    int shots_hit;
    int shots_missed;
} Player;

typedef struct {
    float x, y;
    float angle;
    float speed;
    int hp;
    int active;
    int type;           // define qué patrón de disparo usa
    int fire_timer;     // frames hasta próximo disparo
    int phase;          // para jefes con múltiples fases
    int frame_count;    // frames de vida, útil para patrones
} Enemy;

typedef struct {
    float x, y;
    int active;
    int value;
} Item;

// ================================
// ESTADO DEL JUEGO
// ================================
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER
} GameState;

#endif