#include "stage.h"
#include "enemy.h"
#include "player.h"
#include "game.h"
#include <stdlib.h>
#include <math.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

// ================================
// VARIABLES GLOBALES
// ================================
StageState stage_state;

// ================================
// DEFINICION DE LOS 4 NIVELES
// Cada nivel tiene oleadas distintas
// con enemigos de diferente tipo,
// vida, velocidad y formacion
// ================================
StageDescriptor stages[MAX_STAGES] = {

    // ----------------------------
    // NIVEL 1: "Sector Alfa"
    // Introduccion — enemigos lentos,
    // patrones simples, jefe basico
    // ----------------------------
    {
        "Sector Alfa",
        4,  // 4 oleadas
        {
        // oleada 0: 6 enemigos tipo 0, lentos, linea horizontal
        { 0, 6, 1.2f, 2, 40, 0 },
        // oleada 1: 5 enemigos tipo 1, formation V
        { 1, 5, 1.0f, 2, 50, 1 },
        // oleada 2: 8 enemigos tipo 0, rapidos, linea
        { 0, 8, 1.5f, 3, 30, 0 },
        // oleada 3: 4 enemigos tipo 2 , diamante
        { 2, 4, 1.2f, 3, 45, 2 },
    },
    1,          // tiene jefe
    0,          // jefe tipo 0 
    BOSS_HP_BASE,
    0.5f
},

// ----------------------------
// NIVEL 2: "Mar de Cristal"
// Dificultad media — mezcla de
// patrones, enemigos mas agresivos
// ----------------------------
{
    "Mar de Cristal",
    5,  // 5 oleadas
    {
        // oleada 0: 8 tipo 1 , rapidos
        { 1, 8, 1.4f, 3, 35, 0 },
        // oleada 1: 6 tipo 2 , V
        { 2, 6, 1.3f, 3, 40, 1 },
        // oleada 2: 10 tipo 0 , linea, muchos
        { 0, 10, 1.6f, 2, 25, 0 },
        // oleada 3: 5 tipo 3 , diamante
        { 3, 5, 1.0f, 4, 50, 2 },
        // oleada 4: 6 tipo 2 + diamante, mas rapidos
        { 2, 6, 1.5f, 3, 35, 2 },
    },
    1,          // tiene jefe
    1,          // jefe tipo 1 (circulo — mas peligroso)
    BOSS_HP_BASE + 10,
    0.7f
},

// ----------------------------
// NIVEL 3: "Abismo"
// Difícil — enemigos espiral
// y doble circulo 
{
    "Abismo",
    6,  // 6 oleadas
    {
        // oleada 0: 8 tipo 4 , aleatorio
        { 4, 8, 1.3f, 3, 30, 3 },
        // oleada 1: 6 tipo 3 , V
        { 3, 6, 1.2f, 4, 45, 1 },
        // oleada 2: 10 tipo 1 , linea
        { 1, 10, 1.5f, 3, 25, 0 },
        // oleada 3: 8 tipo 2 , diamante
        { 2, 8, 1.4f, 3, 35, 2 },
        // oleada 4: 6 tipo 4  aleatorio
        { 4, 6, 1.6f, 4, 30, 3 },
        // oleada 5: 5 tipo 3 , V
        { 3, 5, 1.3f, 5, 50, 1 },
    },
    1,          // tiene jefe
    3,          // jefe tipo 3 
    BOSS_HP_BASE + 20,
    0.9f
},

// ----------------------------
// NIVEL 4: "El Void"
// Final — todos los tipos mezclados,
// jefe usa patron espiral 
// ----------------------------
{
    "El Void",
    8,  // 8 oleadas — maximo
    {
        // oleada 0: 10 tipo 0, rapidos
        { 0, 10, 2.0f, 3, 20, 0 },
        // oleada 1: 8 tipo 4 (espiral), V
        { 4, 8, 1.8f, 4, 30, 1 },
        // oleada 2: 6 tipo 3 (doble circulo), aleatorio
        { 3, 6, 1.5f, 5, 40, 3 },
        // oleada 3: 10 tipo 2 (spread), linea
        { 2, 10, 1.7f, 3, 25, 0 },
        // oleada 4: 8 tipo 1 (circulo), diamante
        { 1, 8, 1.6f, 4, 35, 2 },
        // oleada 5: 6 tipo 4 (espiral), aleatorio
        { 4, 6, 2.0f, 5, 30, 3 },
        // oleada 6: 10 tipo 3 (doble circulo), V
        { 3, 10, 1.8f, 4, 35, 1 },
        // oleada 7: mezcla de todo — spread masivo
        { 2, 12, 2.0f, 3, 20, 3 },
    },
    1,          // tiene jefe final
    4,          // jefe tipo 4 (espiral — el mas dificil)
    BOSS_HP_BASE + 40,
    1.2f
}
};

// ================================
// INICIALIZAR EL ESTADO DEL STAGE
// ================================
void init_stage_state() {
    stage_state.current_stage = 0;
    stage_state.current_wave = 0;
    stage_state.second_loop = 0;
    stage_state.wave_timer = 120;  // 2 segundos antes de la primera oleada
    stage_state.spawned_in_wave = 0;
    stage_state.spawn_timer = 0;
    stage_state.boss_active = 0;
    stage_state.boss_idx = -1;
    stage_state.stage_clear = 0;
    stage_state.transition_timer = 0;
}

// ================================
// CONTAR ENEMIGOS ACTIVOS
// ================================
int count_active_enemies() {
    int count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemy_pool[i].active) count++;
    }
    return count;
}

// ================================
// VERIFICAR SI SE LIMPIO EL STAGE
// El stage se limpia cuando no hay
// enemigos activos y se completaron
// todas las oleadas (o murio el jefe)
// ================================
int is_stage_clear() {
    StageDescriptor* sd = &stages[stage_state.current_stage];

    // si tiene jefe, debe haber muerto
    if (sd->has_boss && stage_state.boss_active) return 0;

    // todas las oleadas completadas y sin enemigos
    int all_waves_done = (stage_state.current_wave >= sd->wave_count);
    return all_waves_done && (count_active_enemies() == 0);
}

// ================================
// SPAWN EN FORMACION
// Distribuye los enemigos segun el
// tipo de formacion especificado
// ================================
void spawn_wave_formation(WaveDescriptor* wave) {
    int n = wave->count;

    //multiplicador de dificultad en segunda vuelta
    int hp = wave->hp + (stage_state.second_loop ? 2 : 0);
    float spd = wave->speed * (stage_state.second_loop ? 1.3f : 1.0f);

    float margin = 60.0f;
    float usable = SCREEN_W - 2 * margin;

    for (int i = 0; i < n; i++) {
        float x = 0, y = 0;
        float angle = 1.5708f;  

        switch (wave->formation) {

        case 0: // linea horizontal
            x = margin + (usable / (n - 1 > 0 ? n - 1 : 1)) * i;
            y = -30.0f - i * 5.0f;
            break;

        case 1: // formacion V
        {
            float center = SCREEN_W / 2.0f;
            float half = n / 2;
            float offset = (i - half) * 50.0f;
            x = center + offset;
            y = -30.0f - fabsf(offset) * 0.4f;
        }
        break;

        case 2: // diamante — entran desde los 4 lados
        {
            int side = i % 4;
            float t = (float)(i / 4) * 0.3f;
            if (side == 0) { x = SCREEN_W / 2.0f - 80 - t * 30; y = -30; }
            if (side == 1) { x = SCREEN_W / 2.0f + 80 + t * 30; y = -30; }
            if (side == 2) { x = -30; y = SCREEN_H / 3.0f + t * 40; angle = 0.0f; }
            if (side == 3) { x = SCREEN_W + 30; y = SCREEN_H / 3.0f + t * 40; angle = 3.14159f; }
        }
        break;

        case 3: // aleatorio por los bordes superiores
            x = margin + (float)(rand() % (int)usable);
            y = -30.0f - (float)(rand() % 80);
            break;
        }

        // tipo del enemigo 
        int type = wave->enemy_type;
        if (stage_state.second_loop && type < 4) type++;

        spawn_enemy(x, y, angle, spd, hp, type);
    }
}

// ================================
// AVANZAR A LA SIGUIENTE OLEADA
// ================================
void next_wave() {
    StageDescriptor* sd = &stages[stage_state.current_stage];

    stage_state.current_wave++;
    stage_state.spawned_in_wave = 0;
    stage_state.spawn_timer = 0;

    // si se terminaron las oleadas normales, spawnear jefe
    if (stage_state.current_wave >= sd->wave_count) {
        if (sd->has_boss && !stage_state.boss_active) {
            stage_state.wave_timer = 180;  // 3 segundos antes del jefe
        }
        return;
    }

    stage_state.wave_timer = 150;  // 2.5 segundos entre oleadas
}

// ================================
// SPAWNEAR JEFE DEL NIVEL
// El jefe es un enemigo especial:
// mas grande, mas vida, tipo especial
// ================================
void spawn_boss(int stage_idx) {
    StageDescriptor* sd = &stages[stage_idx];

    int hp = sd->boss_hp + (stage_state.second_loop ? 20 : 0);
    float x = SCREEN_W / 2.0f;
    float y = -60.0f;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemy_pool[i].active) continue;

        enemy_pool[i].x = x;
        enemy_pool[i].y = y;
        enemy_pool[i].angle = 1.5708f;
        enemy_pool[i].speed = 0.8f;   // jefe se mueve lento
        enemy_pool[i].hp = hp;
        enemy_pool[i].active = 1;
        enemy_pool[i].type = sd->boss_type;
        enemy_pool[i].fire_timer = 40;     // dispara mas rapido
        enemy_pool[i].phase = 0;
        enemy_pool[i].frame_count = 0;

        stage_state.boss_idx = i;
        stage_state.boss_active = 1;
        return;
    }
}

// ================================
// AVANZAR AL SIGUIENTE NIVEL
// ================================
void next_stage() {
    stage_state.current_stage++;

    // si supero los 4 niveles, inicia segunda vuelta
    if (stage_state.current_stage >= MAX_STAGES) {
        stage_state.current_stage = 0;
        stage_state.second_loop = 1;
    }

    stage_state.current_wave = 0;
    stage_state.spawned_in_wave = 0;
    stage_state.spawn_timer = 0;
    stage_state.boss_active = 0;
    stage_state.boss_idx = -1;
    stage_state.stage_clear = 0;
    stage_state.wave_timer = 180;  // pausa antes del primer spawn del nivel
    stage_state.transition_timer = 180;  // 3 seg de pantalla de transicion
}

// ================================
// UPDATE PRINCIPAL DEL STAGE
// Se llama una vez por frame desde
// el update() de main.cpp
// ================================
void update_stage() {
    StageDescriptor* sd = &stages[stage_state.current_stage];

    // --- contar transicion de nivel ---
    if (stage_state.transition_timer > 0) {
        stage_state.transition_timer--;
        return;  // no spawneamos nada durante la transicion
    }

    // --- verificar si murio el jefe ---
    if (stage_state.boss_active) {
        if (stage_state.boss_idx >= 0 &&
            !enemy_pool[stage_state.boss_idx].active) {
            // jefe muerto — limpiar y avanzar nivel
            stage_state.boss_active = 0;
            stage_state.stage_clear = 1;
            player.score += 1000 * (stage_state.current_stage + 1);
            stage_state.transition_timer = 240;  // 4 segundos de celebracion
            next_stage();
            return;
        }

        // mientras el jefe vive, moverlo en patron serpentina
        if (stage_state.boss_idx >= 0) {
            Enemy* boss = &enemy_pool[stage_state.boss_idx];
            
            if (boss->y < 80.0f) {
                boss->y += boss->speed;
            }
            else {
                // serpentea horizontalmente usando frame_count
                boss->x = SCREEN_W / 2.0f + sinf(boss->frame_count * 0.02f) * 180.0f;
                // en segunda fase mueve mas rapido
                if (boss->hp <= sd->boss_hp / 2) {
                    boss->x = SCREEN_W / 2.0f + sinf(boss->frame_count * 0.035f) * 200.0f;
                    boss->fire_timer = 25;  // dispara aun mas rapido
                }
            }
        }
        return;  // durante el jefe no spawneamos oleadas normales
    }

    // --- timer entre oleadas ---
    if (stage_state.wave_timer > 0) {
        stage_state.wave_timer--;

        // si es momento del jefe
        if (stage_state.wave_timer == 0 &&
            stage_state.current_wave >= sd->wave_count) {
            if (sd->has_boss) {
                spawn_boss(stage_state.current_stage);
            }
        }
        return;
    }

    // --- si ya se completaron todas las oleadas, esperar al jefe ---
    if (stage_state.current_wave >= sd->wave_count) return;

    // --- spawn individual dentro de la oleada ---
    WaveDescriptor* wave = &sd->waves[stage_state.current_wave];

    if (stage_state.spawned_in_wave < wave->count) {
        if (stage_state.spawn_timer <= 0) {
            // spawn de UN enemigo a la vez usando la formacion
         
            int original_count = wave->count;
            wave->count = 1;
            spawn_wave_formation(wave);
            wave->count = original_count;

            stage_state.spawned_in_wave++;
            stage_state.spawn_timer = (int)wave->spawn_interval;
        }
        else {
            stage_state.spawn_timer--;
        }
    }
    else {
        // oleada completada — esperar a que mueran todos antes de la siguiente
        if (count_active_enemies() == 0) {
            next_wave();
        }
    }
}

// ================================
// DIBUJAR HUD DEL STAGE
// Nombre del nivel arriba a la dcha
// y barra de vida del jefe si esta activo
// ================================
extern ALLEGRO_FONT* font;

void draw_stage_hud() {
    StageDescriptor* sd = &stages[stage_state.current_stage];

    // nombre del nivel
    const char* loop_prefix = stage_state.second_loop ? "[2ND] " : "";
    al_draw_textf(font, al_map_rgb(180, 220, 255),
        SCREEN_W - 10, 10, ALLEGRO_ALIGN_RIGHT,
        "%sNivel %d: %s", loop_prefix,
        stage_state.current_stage + 1, sd->name);

    // oleada actual
    if (!stage_state.boss_active &&
        stage_state.current_wave < sd->wave_count) {
        al_draw_textf(font, al_map_rgb(150, 150, 200),
            SCREEN_W - 10, 28, ALLEGRO_ALIGN_RIGHT,
            "Oleada %d/%d",
            stage_state.current_wave + 1, sd->wave_count);
    }

    // barra de vida del jefe
    if (stage_state.boss_active && stage_state.boss_idx >= 0) {
        Enemy* boss = &enemy_pool[stage_state.boss_idx];
        float  max_hp = (float)(sd->boss_hp + (stage_state.second_loop ? 20 : 0));
        float  ratio = boss->hp / max_hp;
        if (ratio < 0) ratio = 0;

        float bar_x = 80.0f;
        float bar_w = SCREEN_W - 160.0f;
        float bar_y = SCREEN_H - 20.0f;
        float bar_h = 10.0f;

        // fondo
        al_draw_filled_rectangle(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h,
            al_map_rgb(40, 0, 0));
        // vida restante — rojo a amarillo segun porcentaje
        ALLEGRO_COLOR boss_color = (ratio > 0.5f)
            ? al_map_rgb(220, 50, 50)
            : al_map_rgb(255, 200, 0);
        al_draw_filled_rectangle(bar_x, bar_y, bar_x + bar_w * ratio, bar_y + bar_h,
            boss_color);
        // borde
        al_draw_rectangle(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h,
            al_map_rgb(200, 100, 100), 1.0f);

        al_draw_text(font, al_map_rgb(255, 100, 100),
            SCREEN_W / 2.0f, bar_y - 14,
            ALLEGRO_ALIGN_CENTER, "-- JEFE --");
    }

    // pantalla de transicion entre niveles
    if (stage_state.transition_timer > 0 && stage_state.transition_timer < 200) {
        float alpha = (float)stage_state.transition_timer / 200.0f;
        int a = (int)(alpha * 255);
        al_draw_textf(font, al_map_rgba(255, 255, 100, a),
            SCREEN_W / 2.0f, SCREEN_H / 2.0f - 10,
            ALLEGRO_ALIGN_CENTER,
            "Nivel %d completado!", stage_state.current_stage);
    }
}