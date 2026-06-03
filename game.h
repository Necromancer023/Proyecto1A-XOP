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
    int bounces;
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
    int type;           // define que patron de disparo usa
    int fire_timer;     // frames hasta proximo disparo
    int phase;          // para jefes con multiples fases (0 = normal, 1 = segunda fase)
    int frame_count;    // frames de vida, util para patrones como espiral
    int sprite_idx;
} Enemy;

typedef struct {
    float x, y;
    int active;
    int value;
    int type;           // 0 = DAL coin, 1 = Reflect Shield Slot (bomba extra)
    int anim_frame;
    int anim_timer;
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
    MODE_MANIAC = 2,  // accion frenetica (FPS y velocidad aumentados)
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
    // bonus de HP segun dificultad
    int bonus[8] = { -1, 0, 0, 1, 1, 2, 3, 5 };
    return bonus[difficulty];
}

static inline int diff_fire_rate_bonus() {
    // reduccion de frames entre disparos (mas negativo = mas rapido)
    int bonus[8] = { 20, 10, 0, -5, -10, -15, -20, -25 };
    return bonus[difficulty];
}

// ================================
// AUDIO/SPRITES — declaraciones externas
// ================================
#include <allegro5/allegro_audio.h>
extern ALLEGRO_SAMPLE* sfx_shoot;
extern ALLEGRO_SAMPLE* sfx_explode;
extern ALLEGRO_SAMPLE* sfx_pickup;
extern ALLEGRO_BITMAP* spr_player;
extern ALLEGRO_BITMAP* spr_boss1;
extern ALLEGRO_BITMAP* spr_boss2;
extern ALLEGRO_BITMAP* spr_enemies[5];
extern ALLEGRO_BITMAP* spr_coin[8];

#endif