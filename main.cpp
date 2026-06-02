#define _CRT_SECURE_NO_WARNINGS
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include "game.h"
#include "bullet.h"
#include "player.h"
#include "enemy.h"
#include "collision.h"
#include "stats.h"
#include "item.h"
#include "stage.h"
#include "screens.h"
#include "weapons.h"

// ================================
// VARIABLES GLOBALES
// ================================
ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_TIMER* timer = NULL;
ALLEGRO_EVENT_QUEUE* queue = NULL;
ALLEGRO_FONT* font = NULL;

// Modo y dificultad — globales del juego
GameMode game_mode = MODE_ORIGINAL;
int      difficulty = 2;             // Normal por default

GameState game_state = STATE_MENU;

// ================================
// AUDIO
// ================================
static ALLEGRO_SAMPLE* sfx_shoot = NULL;
static ALLEGRO_SAMPLE* sfx_explode = NULL;
static ALLEGRO_SAMPLE* sfx_pickup = NULL;
static ALLEGRO_SAMPLE_INSTANCE* music_inst = NULL;
static ALLEGRO_SAMPLE* music_sample = NULL;

static void load_audio() {
    
    sfx_shoot = al_load_sample("assets/shoot.wav");
    sfx_explode = al_load_sample("assets/explode.wav");
    sfx_pickup = al_load_sample("assets/pickup.wav");
    music_sample = al_load_sample("assets/bgm.ogg");

    if (music_sample) {
        music_inst = al_create_sample_instance(music_sample);
        al_attach_sample_instance_to_mixer(music_inst, al_get_default_mixer());
        al_set_sample_instance_playmode(music_inst, ALLEGRO_PLAYMODE_LOOP);
        al_play_sample_instance(music_inst);
    }
}

static void unload_audio() {
    if (music_inst) { al_stop_sample_instance(music_inst); al_destroy_sample_instance(music_inst); }
    if (music_sample) al_destroy_sample(music_sample);
    if (sfx_shoot)    al_destroy_sample(sfx_shoot);
    if (sfx_explode)  al_destroy_sample(sfx_explode);
    if (sfx_pickup)   al_destroy_sample(sfx_pickup);
}

// ================================
// INICIAR UNA PARTIDA NUEVA
// Resetea todos los sistemas
// ================================
static void start_new_game() {
    init_bullet_pool();
    init_enemy_pool();
    init_item_pool();
    init_player();
    init_stage_state();
    init_secondary(WEAPON_BOTS);  // arma secundaria inicial

    // aplicar modo de juego al jugador
    switch (game_mode) {
    case MODE_SLUDGE:
        al_set_timer_speed(timer, 2.0 / FPS);  // mitad de velocidad
        break;
    case MODE_MANIAC:
        al_set_timer_speed(timer, 0.7 / FPS);  // mas rapido
        player.speed = PLAYER_SPEED * 1.4f;
        break;
    default:
        al_set_timer_speed(timer, 1.0 / FPS);
        break;
    }

    game_state = STATE_PLAYING;
}

// ================================
// DIBUJAR HUD EN JUEGO
// ================================
static void draw_hud() {
    // --- columna izquierda ---
    al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 10, 0,
        "HP: %d", player.hp);
    al_draw_textf(font, al_map_rgb(200, 200, 255), 10, 28, 0,
        "Bombas: %d", player.bombs);
    al_draw_textf(font, al_map_rgb(255, 230, 50), 10, 46, 0,
        "Score: %d", player.score);
    al_draw_textf(font, al_map_rgb(0, 255, 150), 10, 64, 0,
        "Combo: x%d", player.combo);
    al_draw_textf(font, al_map_rgb(150, 150, 255), 10, 82, 0,
        "Arma 1: %d", player.weapon);

    // estadisticas de disparo (debajo)
    al_draw_textf(font, al_map_rgb(150, 150, 150), 10, 100, 0,
        "D:%d A:%d F:%d",
        player.shots_fired, player.shots_hit, player.shots_missed);

    // modo de juego si no es Original
    if (game_mode != MODE_ORIGINAL) {
        const char* modes[] = { "", "SLUDGE", "MANIAC", "MASSACRE" };
        al_draw_text(font, al_map_rgb(255, 150, 50),
            SCREEN_W / 2.0f, SCREEN_H - 15,
            ALLEGRO_ALIGN_CENTER, modes[game_mode]);
    }

    // HUD del stage (nombre, oleada, barra de jefe)
    draw_stage_hud();

    // HUD del arma secundaria
    draw_secondary();
}

// ================================
// DIBUJAR SEGUN ESTADO
// ================================
static void draw() {
    al_clear_to_color(al_map_rgb(0, 0, 20));

    switch (game_state) {

    case STATE_MENU:
        draw_menu();
        break;

    case STATE_OPTIONS:
        draw_options();
        break;

    case STATE_PLAYING:
    case STATE_PAUSED:
        draw_bullets();
        draw_enemies();
        draw_items();
        draw_secondary();
        draw_player();
        draw_hud();
        if (game_state == STATE_PAUSED) {
            
            al_draw_filled_rectangle(0, 0, SCREEN_W, SCREEN_H,
                al_map_rgba(0, 0, 0, 100));
            al_draw_text(font, al_map_rgb(255, 255, 100),
                SCREEN_W / 2.0f, SCREEN_H / 2.0f - 20,
                ALLEGRO_ALIGN_CENTER, "PAUSA");
            al_draw_text(font, al_map_rgb(150, 150, 150),
                SCREEN_W / 2.0f, SCREEN_H / 2.0f + 10,
                ALLEGRO_ALIGN_CENTER, "P: continuar  |  Esc: menu");
        }
        break;

    case STATE_GAME_OVER:
        draw_game_over();
        break;

    case STATE_VICTORY:
        draw_victory();
        break;

    case STATE_NAME_ENTRY:
        draw_name_entry();
        break;

    case STATE_STATS:
        draw_stats_screen();
        break;

    default:
        break;
    }

    al_flip_display();
}

// ================================
// ACTUALIZAR LOGICA DE JUEGO
// ================================
static void update() {
    if (game_state != STATE_PLAYING) return;

    // --- multiplicador de modo Massacre

    update_player();
    update_enemies();
    update_bullets();
    update_items();
    update_secondary();         // actualizar armas secundarias (bots, implosion)
    update_stage();             // oleadas, jefes, transiciones de nivel
    check_player_hit();
    check_enemy_hit();

    // --- verificar game over ---
    if (player.hp <= 0) {
        game_state = STATE_GAME_OVER;
    }

    // --- verificar victoria (todos los niveles completados) ---

}

// ================================
// MANEJAR EVENTOS DE TECLADO
// ================================
static void handle_key(ALLEGRO_EVENT* ev) {
    // pantallas que manejan sus propios eventos
    switch (game_state) {
    case STATE_MENU:       update_menu(ev);        return;
    case STATE_OPTIONS:    update_options(ev);     return;
    case STATE_GAME_OVER:  update_game_over(ev);   return;
    case STATE_VICTORY:    update_victory(ev);     return;
    case STATE_NAME_ENTRY: update_name_entry(ev);  return;
    case STATE_STATS:      update_stats_screen(ev); return;
    default: break;
    }

    // eventos globales en juego
    if (ev->type != ALLEGRO_EVENT_KEY_DOWN) return;

    switch (ev->keyboard.keycode) {
    case ALLEGRO_KEY_P:
        if (game_state == STATE_PLAYING)
            game_state = STATE_PAUSED;
        else if (game_state == STATE_PAUSED)
            game_state = STATE_PLAYING;
        break;

    case ALLEGRO_KEY_ESCAPE:
        if (game_state == STATE_PLAYING || game_state == STATE_PAUSED) {
            // guardar partida actual antes de salir al menu
            if (player.score > 0) {
                save_current_game("Abandonado");
            }
            al_set_timer_speed(timer, 1.0 / FPS);  // resetear timer si estaba en modo especial
            game_state = STATE_MENU;
        }
        break;
    }
}

// ================================
// MAIN
// ================================
int main() {
    // --- init allegro ---
    al_init();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_audio();
    al_init_acodec_addon();
    al_install_keyboard();

    // --- crear ventana, timer y cola ---
    display = al_create_display(SCREEN_W, SCREEN_H);
    timer = al_create_timer(1.0 / FPS);
    queue = al_create_event_queue();
    font = al_create_builtin_font();

    al_set_window_title(display, "XOP — Bullet Hell");
    al_reserve_samples(8);

    // --- registrar fuentes de eventos ---
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());

    // --- cargar audio ---
    load_audio();

    // --- inicializar pools (para que esten listos desde el menu) ---
    init_bullet_pool();
    init_enemy_pool();
    init_item_pool();
    init_player();
    init_stage_state();
    init_secondary(WEAPON_BOTS);

    al_start_timer(timer);

    // --- game loop ---
    bool running = true;
    bool redraw = false;

    while (running) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            update();
            redraw = true;
        }
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN ||
            ev.type == ALLEGRO_EVENT_KEY_CHAR) {
            handle_key(&ev);

            // NAME_ENTRY
            if (game_state == STATE_NAME_ENTRY &&
                ev.type == ALLEGRO_EVENT_KEY_CHAR) {
                update_name_entry(&ev);
            }
        }

        // iniciar nueva partida cuando se pide desde las pantallas
        if (game_state == STATE_PLAYING &&
            stage_state.current_stage == 0 &&
            stage_state.current_wave == 0 &&
            count_active_enemies() == 0 &&
            stage_state.wave_timer == 120 &&
            player.score == 0) {
            
        }

        // salir si se pidio desde el menu
        if (game_state == STATE_QUIT) {
            running = false;
        }

        if (redraw && al_is_event_queue_empty(queue)) {
            draw();
            redraw = false;
        }
    }

    // --- guardar estadisticas al cerrar ---
    if (player.score > 0 && name_entry_done == 0) {
        save_current_game("Jugador");
    }

    // --- liberar recursos ---
    unload_audio();
    free_tree(score_tree);
    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);

    return 0;
}