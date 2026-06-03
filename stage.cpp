#include "stage.h"
#include "enemy.h"
#include "player.h"
#include "stats.h"
#include "game.h"
#include <stdlib.h>
#include <math.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

// ================================
// VARIABLES GLOBALES
// ================================
StageState stage_state;

extern GameState game_state;

// ================================
// DEFINICION DE LOS 4 NIVELES
// ================================
StageDescriptor stages[MAX_STAGES] = {

    // ----------------------------
    // NIVEL 1: "Sector Alfa"
    // ----------------------------
    {
        "Sector Alfa",
        4,
        {
            { 0, 6, 1.2f, 2, 40, 0 },
            { 1, 5, 1.0f, 2, 50, 1 },
            { 0, 8, 1.5f, 3, 30, 0 },
            { 2, 4, 1.2f, 3, 45, 2 },
        },
        1, 0, BOSS_HP_BASE, 0.5f
    },

    // ----------------------------
    // NIVEL 2: "Mar de Cristal"
    // ----------------------------
    {
        "Mar de Cristal",
        5,
        {
            { 1, 8,  1.4f, 3, 35, 0 },
            { 2, 6,  1.3f, 3, 40, 1 },
            { 0, 10, 1.6f, 2, 25, 0 },
            { 3, 5,  1.0f, 4, 50, 2 },
            { 2, 6,  1.5f, 3, 35, 2 },
        },
        1, 1, BOSS_HP_BASE + 10, 0.7f
    },

    // ----------------------------
    // NIVEL 3: "Abismo"
    // ----------------------------
    {
        "Abismo",
        6,
        {
            { 4, 8,  1.3f, 3, 30, 3 },
            { 3, 6,  1.2f, 4, 45, 1 },
            { 1, 10, 1.5f, 3, 25, 0 },
            { 2, 8,  1.4f, 3, 35, 2 },
            { 4, 6,  1.6f, 4, 30, 3 },
            { 3, 5,  1.3f, 5, 50, 1 },
        },
        1, 3, BOSS_HP_BASE + 20, 0.9f
    },

    // ----------------------------
    // NIVEL 4: "El Void"
    // ----------------------------
    {
        "El Void",
        8,
        {
            { 0, 10, 2.0f, 3, 20, 0 },
            { 4, 8,  1.8f, 4, 30, 1 },
            { 3, 6,  1.5f, 5, 40, 3 },
            { 2, 10, 1.7f, 3, 25, 0 },
            { 1, 8,  1.6f, 4, 35, 2 },
            { 4, 6,  2.0f, 5, 30, 3 },
            { 3, 10, 1.8f, 4, 35, 1 },
            { 2, 12, 2.0f, 3, 20, 3 },
        },
        1, 4, BOSS_HP_BASE + 40, 1.2f
    }
};

// ================================
// INICIALIZAR EL ESTADO DEL STAGE
// ================================
void init_stage_state() {
    stage_state.current_stage = 0;
    stage_state.current_wave = 0;
    stage_state.second_loop = 0;
    stage_state.wave_timer = 120;
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
// ================================
int is_stage_clear() {
    StageDescriptor* sd = &stages[stage_state.current_stage];

    if (sd->has_boss && stage_state.boss_active) return 0;

    int all_waves_done = (stage_state.current_wave >= sd->wave_count);
    return all_waves_done && (count_active_enemies() == 0);
}

// ================================
// SPAWN EN FORMACION
// ================================
void spawn_wave_formation(WaveDescriptor* wave) {
    int   n = wave->count;
    int   hp = wave->hp + (stage_state.second_loop ? 2 : 0);
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

        case 2: // diamante
        {
            int   side = i % 4;
            float t = (float)(i / 4) * 0.3f;
            if (side == 0) { x = SCREEN_W / 2.0f - 80 - t * 30; y = -30;              angle = 1.5708f; }
            if (side == 1) { x = SCREEN_W / 2.0f + 80 + t * 30; y = -30;              angle = 1.5708f; }
            if (side == 2) { x = -30;           y = SCREEN_H / 3.0f + t * 40; angle = 0.0f; }
            if (side == 3) { x = SCREEN_W + 30; y = SCREEN_H / 3.0f + t * 40; angle = 3.14159f; }
        }
        break;

        case 3: // aleatorio por bordes 
            x = margin + (float)(rand() % (int)usable);
            y = -30.0f - (float)(rand() % 80);
            break;
        }

        int type = wave->enemy_type;
        if (stage_state.second_loop && type < 4) type++;

        spawn_enemy(x, y, angle, spd, hp, type);
    }
}

// ================================
// AVANZAR A LA SIGUIENTE OLEADA
// Guarda progreso
// ================================
void next_wave() {
    StageDescriptor* sd = &stages[stage_state.current_stage];

    stage_state.current_wave++;
    stage_state.spawned_in_wave = 0;
    stage_state.spawn_timer = 0;

    // guardar progreso al cambiar de oleada
    if (player.score > 0) {
        save_current_game("En progreso");
    }

    if (stage_state.current_wave >= sd->wave_count) {
        if (sd->has_boss && !stage_state.boss_active) {
            stage_state.wave_timer = 180;
        }
        return;
    }

    stage_state.wave_timer = 150;
}

// ================================
// SPAWNEAR JEFE DEL NIVEL
// ================================
void spawn_boss(int stage_idx) {
    StageDescriptor* sd = &stages[stage_idx];

    int   hp = sd->boss_hp + (stage_state.second_loop ? 20 : 0);
    float x = SCREEN_W / 2.0f;
    float y = -60.0f;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemy_pool[i].active) continue;

        enemy_pool[i].x = x;
        enemy_pool[i].y = y;
        enemy_pool[i].angle = 1.5708f;
        enemy_pool[i].speed = 0.8f;
        enemy_pool[i].hp = hp;
        enemy_pool[i].active = 1;
        enemy_pool[i].type = sd->boss_type;
        enemy_pool[i].fire_timer = 40;
        enemy_pool[i].phase = 0;
        enemy_pool[i].frame_count = 0;

        stage_state.boss_idx = i;
        stage_state.boss_active = 1;

        // guardar el HP maximo del jefe para saber cuando activar fase 2
       
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
        // segunda vuelta completada VICTORIA
        if (stage_state.second_loop) {
            game_state = STATE_VICTORY;
            return;
        }
        stage_state.current_stage = 0;
        stage_state.second_loop = 1;
    }

    stage_state.current_wave = 0;
    stage_state.spawned_in_wave = 0;
    stage_state.spawn_timer = 0;
    stage_state.boss_active = 0;
    stage_state.boss_idx = -1;
    stage_state.stage_clear = 0;
    stage_state.wave_timer = 180;
    stage_state.transition_timer = 180;
}

// ================================
// UPDATE PRINCIPAL DEL STAGE
// ================================
void update_stage() {
    StageDescriptor* sd = &stages[stage_state.current_stage];

    // transicion entre niveles
    if (stage_state.transition_timer > 0) {
        stage_state.transition_timer--;
        return;
    }

    // verificar si murio el jefe
    if (stage_state.boss_active) {
        if (stage_state.boss_idx >= 0 &&
            !enemy_pool[stage_state.boss_idx].active) {

            // jefe muerto
            stage_state.boss_active = 0;
            stage_state.stage_clear = 1;
            player.score += 1000 * (stage_state.current_stage + 1);
            stage_state.transition_timer = 240;
            next_stage();
            return;
        }

        // jefe vivo entonces moverlo y gestionar fase 2
        if (stage_state.boss_idx >= 0) {
            Enemy* boss = &enemy_pool[stage_state.boss_idx];
            int    boss_hp_max = sd->boss_hp + (stage_state.second_loop ? 20 : 0);

            // --- transicion a fase 2 ---
            // se activa cuando el jefe pierde la mitad de su vida
            if (boss->phase == 0 && boss->hp <= boss_hp_max / 2) {
                boss->phase = 1;
                // en fase 2 el jefe dispara bastante mas rapido
                boss->fire_timer = 20;
            }

            // fase 3 exclusiva del jefe 2
            if (stage_state.current_stage == 1 &&
                boss->phase == 1 && boss->hp <= boss_hp_max / 4) {
                boss->phase = 2;
                boss->fire_timer = 15;
            }

            // --- movimiento del jefe ---
            float stop_y = 80.0f;
            if (stage_state.current_stage == 1) stop_y = 150.0f;  // jefe 2 mas abajo

            if (boss->y < stop_y) {
                boss->y += boss->speed;
            }
            else {
                // serpentina horizontal
                float freq = (boss->phase >= 1) ? 0.035f : 0.02f;
                float amp = (boss->phase >= 1) ? 200.0f : 180.0f;
                boss->x = SCREEN_W / 2.0f + sinf(boss->frame_count * freq) * amp;

                // en fase 2 tambien se mueve verticalmente
                if (boss->phase >= 1) {
                    float base_y = (stage_state.current_stage == 1) ? 150.0f : 80.0f;
                    boss->y = base_y + sinf(boss->frame_count * 0.018f) * 40.0f;
                }
            }
        }
        return;
    }

    // timer entre oleadas
    if (stage_state.wave_timer > 0) {
        stage_state.wave_timer--;

        if (stage_state.wave_timer == 0 &&
            stage_state.current_wave >= sd->wave_count) {
            if (sd->has_boss) {
                spawn_boss(stage_state.current_stage);
            }
        }
        return;
    }

    // todas las oleadas completadas, esperar al jefe
    if (stage_state.current_wave >= sd->wave_count) return;

    // spawn individual dentro de la oleada
    WaveDescriptor* wave = &sd->waves[stage_state.current_wave];

    if (stage_state.spawned_in_wave < wave->count) {
        if (stage_state.spawn_timer <= 0) {
            // spawnear UN enemigo a la vez en la posicion correcta de la formacion
            // Calculamos la posicion manualmente 
            int   i = stage_state.spawned_in_wave;
            int   n = wave->count;
            int   hp = wave->hp + (stage_state.second_loop ? 2 : 0);
            float spd = wave->speed * (stage_state.second_loop ? 1.3f : 1.0f);
            float margin = 60.0f;
            float usable = SCREEN_W - 2 * margin;
            float x = 0, y = 0;
            float angle = 1.5708f;
            int   type = wave->enemy_type;
            if (stage_state.second_loop && type < 4) type++;

            switch (wave->formation) {
            case 0:
                x = margin + (usable / (n - 1 > 0 ? n - 1 : 1)) * i;
                y = -30.0f - i * 5.0f;
                break;
            case 1:
            {
                float center = SCREEN_W / 2.0f;
                float half = n / 2;
                float offset = (i - half) * 50.0f;
                x = center + offset;
                y = -30.0f - fabsf(offset) * 0.4f;
            }
            break;
            case 2:
            {
                int   side = i % 4;
                float t = (float)(i / 4) * 0.3f;
                if (side == 0) { x = SCREEN_W / 2.0f - 80 - t * 30; y = -30;              angle = 1.5708f; }
                if (side == 1) { x = SCREEN_W / 2.0f + 80 + t * 30; y = -30;              angle = 1.5708f; }
                if (side == 2) { x = -30;           y = SCREEN_H / 3.0f + t * 40; angle = 0.0f; }
                if (side == 3) { x = SCREEN_W + 30; y = SCREEN_H / 3.0f + t * 40; angle = 3.14159f; }
            }
            break;
            case 3:
                x = margin + (float)(rand() % (int)usable);
                y = -30.0f - (float)(rand() % 80);
                break;
            }

            spawn_enemy(x, y, angle, spd, hp, type);

            stage_state.spawned_in_wave++;
            stage_state.spawn_timer = (int)wave->spawn_interval;
        }
        else {
            stage_state.spawn_timer--;
        }
    }
    else {
        // oleada completada, esperar a que mueran todos
        if (count_active_enemies() == 0) {
            next_wave();
        }
    }
}

// ================================
// DIBUJAR HUD DEL STAGE
// ================================
extern ALLEGRO_FONT* font;

void draw_stage_hud() {
    StageDescriptor* sd = &stages[stage_state.current_stage];

    const char* loop_prefix = stage_state.second_loop ? "[2ND] " : "";
    al_draw_textf(font, al_map_rgb(180, 220, 255),
        SCREEN_W - 10, 10, ALLEGRO_ALIGN_RIGHT,
        "%sNivel %d: %s", loop_prefix,
        stage_state.current_stage + 1, sd->name);

    if (!stage_state.boss_active &&
        stage_state.current_wave < sd->wave_count) {
        al_draw_textf(font, al_map_rgb(150, 150, 200),
            SCREEN_W - 10, 28, ALLEGRO_ALIGN_RIGHT,
            "Oleada %d/%d",
            stage_state.current_wave + 1, sd->wave_count);
    }

    // barra de vida del jefe + indicador de fase
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

        // color segun fase
        ALLEGRO_COLOR boss_bar_col = (boss->phase == 1)
            ? al_map_rgb(220, 50, 220)   
            : ((ratio > 0.5f)
                ? al_map_rgb(220, 50, 50)
                : al_map_rgb(255, 200, 0));

        al_draw_filled_rectangle(bar_x, bar_y,
            bar_x + bar_w * ratio, bar_y + bar_h,
            boss_bar_col);
        al_draw_rectangle(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h,
            al_map_rgb(200, 100, 100), 1.0f);

        // texto: muestra la fase actual
        const char* boss_label = (boss->phase == 2) ? "-- JEFE FASE 3 --" :
                                 (boss->phase == 1) ? "-- JEFE FASE 2 --" : "-- JEFE --";
        al_draw_text(font, al_map_rgb(255, 100, 100),
            SCREEN_W / 2.0f, bar_y - 14,
            ALLEGRO_ALIGN_CENTER, boss_label);
    }

    // pantalla de transicion entre niveles
    if (stage_state.transition_timer > 0 && stage_state.transition_timer < 200) {
        float alpha = (float)stage_state.transition_timer / 200.0f;
        int   a = (int)(alpha * 255);

        // si es victoria (completamos todos los niveles y el segundo loop)
        const char* msg = (stage_state.current_stage == 0 && stage_state.second_loop == 0 &&
            stage_state.stage_clear)
            ? "Nivel 4 completado! Iniciando 2da vuelta..."
            : "Nivel completado!";

        al_draw_textf(font, al_map_rgba(255, 255, 100, a),
            SCREEN_W / 2.0f, SCREEN_H / 2.0f - 10,
            ALLEGRO_ALIGN_CENTER, "%s", msg);
    }
}