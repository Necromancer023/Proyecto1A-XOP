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
    STATE_OPTIONS,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_VICTORY,
    STATE_NAME_ENTRY,
    STATE_STATS,
    STATE_QUIT
} GameState;

// ================================
// MODOS DE JUEGO
// ================================
typedef enum {
    MODE_ORIGINAL = 0,  // velocidad normal
    MODE_SLUDGE = 1,  // camara lenta (FPS reducido a la mitad)
    MODE_MANIAC = 2,  // accion frenética (FPS y velocidad aumentados)
    MODE_MASSACRE = 3   // doble cantidad de enemigos spawneados
} GameMode;

// ================================
// DIFICULTAD GLOBAL
// afecta velocidad de balas enemigas,
// HP de enemigos y patrones del jefe
// ================================
extern int      difficulty;   // 0-7
extern GameMode game_mode;

// ================================
// UTILIDAD
// ================================
static inline float diff_bullet_speed() {
    // de 0.7x (Easiest) a 2.0x (Insane)
    float factors[8] = { 0.7f, 0.85f, 1.0f, 1.15f, 1.3f, 1.5f, 1.75f, 2.0f };
    return factors[difficulty];
}

static inline int diff_enemy_hp_bonus() {
    // bonus de HP según dificultad
    int bonus[8] = { -1, 0, 0, 1, 1, 2, 3, 5 };
    return bonus[difficulty];
}

static inline int diff_fire_rate_bonus() {
    // reduccion de frames entre disparos (más negativo = más rápido)
    int bonus[8] = { 20, 10, 0, -5, -10, -15, -20, -25 };
    return bonus[difficulty];
}

#endif