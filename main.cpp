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

// ================================
// VARIABLES GLOBALES
// ================================
ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_TIMER* timer = NULL;
ALLEGRO_EVENT_QUEUE* queue = NULL;
ALLEGRO_FONT* font = NULL;

GameState game_state = STATE_PLAYING;

// ================================
// DIBUJAR HUD
// ================================
void draw_hud() {
    // vida
    al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 10, 0,
        "HP: %d", player.hp);

    // bombas
    al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 30, 0,
        "Bombas: %d", player.bombs);

    // puntaje
    al_draw_textf(font, al_map_rgb(255, 255, 0), 10, 50, 0,
        "Score: %d", player.score);

    // combo
    al_draw_textf(font, al_map_rgb(0, 255, 150), 10, 70, 0,
        "Combo: %d", player.combo);

    // arma actual
    al_draw_textf(font, al_map_rgb(150, 150, 255), 10, 90, 0,
        "Arma: %d", player.weapon);

    // estadisticas de disparo
    al_draw_textf(font, al_map_rgb(200, 200, 200), 10, 110, 0,
        "Disparos: %d | Aciertos: %d | Fallos: %d",
        player.shots_fired,
        player.shots_hit,
        player.shots_missed);
}

// ================================
// DIBUJAR SEGUN ESTADO
// ================================
void draw() {
    al_clear_to_color(al_map_rgb(0, 0, 20));

    switch (game_state) {
    case STATE_PLAYING:
        draw_bullets();
        draw_enemies();
        draw_player();
        draw_hud();
        break;

    case STATE_PAUSED:
        draw_bullets();
        draw_enemies();
        draw_player();
        draw_hud();
        al_draw_text(font, al_map_rgb(255, 255, 0),
            SCREEN_W / 2, SCREEN_H / 2,
            ALLEGRO_ALIGN_CENTER, "PAUSA");
        break;

    case STATE_GAME_OVER:
        al_draw_text(font, al_map_rgb(255, 50, 50),
            SCREEN_W / 2, SCREEN_H / 2 - 20,
            ALLEGRO_ALIGN_CENTER, "GAME OVER");
        al_draw_textf(font, al_map_rgb(255, 255, 255),
            SCREEN_W / 2, SCREEN_H / 2 + 10,
            ALLEGRO_ALIGN_CENTER, "Score final: %d", player.score);
        break;

    case STATE_MENU:
        al_draw_text(font, al_map_rgb(255, 255, 255),
            SCREEN_W / 2, SCREEN_H / 2,
            ALLEGRO_ALIGN_CENTER, "XOP — Presiona ENTER para jugar");
        break;
    }

    al_flip_display();
}

// ================================
// ACTUALIZAR LOGICA
// ================================
void update() {
    if (game_state != STATE_PLAYING) return;

    update_player();
    update_enemies();
    update_bullets();
    check_player_hit();
    check_enemy_hit();

    // aqui va conecta update_stage() para hacer los stages
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

    al_set_window_title(display, "XOP");

    // --- registrar fuentes de eventos ---
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());

    // --- inicializar sistemas ---
    init_bullet_pool();
    init_enemy_pool();
    init_player();

    // --- spawn de prueba para verificar que todo funciona ---
    spawn_enemy(100, 50, 1.57f, 1.5f, 5, 0);  // tipo 0 = apuntado
    spawn_enemy(240, 50, 1.57f, 1.0f, 5, 1);  // tipo 1 = circulo
    spawn_enemy(380, 50, 1.57f, 1.5f, 5, 2);  // tipo 2 = spread

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
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            // pausa
            if (ev.keyboard.keycode == ALLEGRO_KEY_P) {
                if (game_state == STATE_PLAYING)
                    game_state = STATE_PAUSED;
                else if (game_state == STATE_PAUSED)
                    game_state = STATE_PLAYING;
            }
            // salir
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                running = false;
            }
        }

        if (redraw && al_is_event_queue_empty(queue)) {
            draw();
            redraw = false;
        }
    }

    // --- guardar estadisticas al salir ---
    save_current_game("Jugador1");
    show_top_scores();

    // --- liberar memoria ---
    free_tree(score_tree);
    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);

    return 0;
}