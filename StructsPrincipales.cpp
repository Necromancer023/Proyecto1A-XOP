// Proyectil (usado tanto por jugador como enemigos)
typedef struct {
    float x, y;
    float angle;   // en radianes
    float speed;
    int active;
    int owner;     // 0 = jugador, 1 = enemigo
} Bullet;

// Jugador
typedef struct {
    float x, y;
    float speed;
    int hp;
    int bombs;
    int weapon;         // 1 al 6
    int score;
    int combo;
    // estadísticas
    int shots_fired;
    int shots_hit;
    int shots_missed;
} Player;

// Enemigo
typedef struct {
    float x, y;
    float angle;
    float speed;
    int hp;
    int active;
    int type;           // determina el patrón de disparo
    int fire_timer;     // frames hasta próximo disparo
    int phase;          // para jefes con múltiples fases
} Enemy;