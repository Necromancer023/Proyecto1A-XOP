#define _CRT_SECURE_NO_WARNINGS
#include "screens.h"
#include "player.h"
#include "stats.h"
#include "stage.h"
#include "game.h"
#include "game_flow.h"
#include <stdio.h>
#include <string.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

// ================================
// REFERENCIAS EXTERNAS
// ================================
extern ALLEGRO_FONT* font;
extern GameState      game_state;
extern GameMode       game_mode;
extern int            difficulty;

// ================================
// VARIABLES COMPARTIDAS
// ================================
char name_entry_buffer[50] = { 0 };
int  name_entry_len = 0;
int  name_entry_done = 0;

// opciones de menu seleccionada
static int menu_sel = 0;
static int options_sel = 0;

// ================================
// INIT
// ================================
void screens_init() {
    menu_sel = 0;
    options_sel = 0;
    name_entry_len = 0;
    name_entry_done = 0;
    memset(name_entry_buffer, 0, sizeof(name_entry_buffer));
}

// ================================
// HELPERS DE DIBUJO
// ================================
static void draw_title(const char* text, float y) {
    // sombra
    al_draw_text(font, al_map_rgb(0, 80, 160),
        SCREEN_W / 2.0f + 2, y + 2, ALLEGRO_ALIGN_CENTER, text);
    al_draw_text(font, al_map_rgb(100, 200, 255),
        SCREEN_W / 2.0f, y, ALLEGRO_ALIGN_CENTER, text);
}

static void draw_option(const char* text, float y, int selected) {
    ALLEGRO_COLOR col = selected
        ? al_map_rgb(255, 230, 50)
        : al_map_rgb(180, 180, 200);
    if (selected) {
        // indicador de seleccion
        al_draw_text(font, al_map_rgb(255, 230, 50),
            SCREEN_W / 2.0f - 80, y, ALLEGRO_ALIGN_CENTER, ">");
    }
    al_draw_text(font, col, SCREEN_W / 2.0f, y, ALLEGRO_ALIGN_CENTER, text);
}

// ================================
// MENU PRINCIPAL
// ================================
static const char* menu_items[] = {
    "Jugar",
    "Opciones",
    "Top Scores",
    "Salir"
};
#define MENU_COUNT 4

void draw_menu() {
    // fondo — cielo estrellado simple con primitivas
    al_clear_to_color(al_map_rgb(0, 0, 20));

    // titulo del juego
    draw_title("XOP", SCREEN_H / 2.0f - 120);
    al_draw_text(font, al_map_rgb(100, 150, 200),
        SCREEN_W / 2.0f, SCREEN_H / 2.0f - 90,
        ALLEGRO_ALIGN_CENTER, "Bullet Hell Shooter");

    // opciones
    for (int i = 0; i < MENU_COUNT; i++) {
        draw_option(menu_items[i],
            SCREEN_H / 2.0f - 20 + i * 28,
            i == menu_sel);
    }

    // controles hint
    al_draw_text(font, al_map_rgb(80, 80, 100),
        SCREEN_W / 2.0f, SCREEN_H - 30,
        ALLEGRO_ALIGN_CENTER, "Flechas: navegar  |  Enter: seleccionar");
}

void update_menu(ALLEGRO_EVENT* ev) {
    if (ev->type != ALLEGRO_EVENT_KEY_DOWN) return;

    switch (ev->keyboard.keycode) {
    case ALLEGRO_KEY_UP:
        menu_sel = (menu_sel - 1 + MENU_COUNT) % MENU_COUNT;
        break;
    case ALLEGRO_KEY_DOWN:
        menu_sel = (menu_sel + 1) % MENU_COUNT;
        break;
    case ALLEGRO_KEY_ENTER:
    case ALLEGRO_KEY_Z:
        switch (menu_sel) {
        case 0: // Jugar — primero pedir nombre, luego arrancar
            screens_init();
            game_state = STATE_NAME_ENTRY;
            break;
        case 1: // Opciones
            game_state = STATE_OPTIONS;
            break;
        case 2: // Top Scores
            game_state = STATE_STATS;
            break;
        case 3: // Salir
            game_state = STATE_QUIT;
            break;
        }
        break;
    case ALLEGRO_KEY_ESCAPE:
        game_state = STATE_QUIT;
        break;
    }
}

// ================================
// OPCIONES
// ================================
static const char* mode_names[] = { "Original", "Sludge", "Maniac", "Massacre" };
static const char* diff_names[] = {
    "Easiest", "Easy", "Normal", "Hard",
    "Very Hard", "Expert", "Lunatic", "Insane"
};
static const char* speed_names[] = { "Lenta", "Media", "Rapida" };

static const char* options_labels[] = {
    "Modo de juego:",
    "Dificultad:   ",
    "Vel. nave:    ",
    "Volver"
};
#define OPTIONS_COUNT 4

void draw_options() {
    al_clear_to_color(al_map_rgb(0, 0, 20));
    draw_title("Opciones", 40);

    // modo de juego
    draw_option(options_labels[0], 130, options_sel == 0);
    al_draw_text(font, al_map_rgb(100, 255, 200),
        SCREEN_W / 2.0f + 30, 130, 0, mode_names[game_mode]);

    // dificultad
    draw_option(options_labels[1], 160, options_sel == 1);
    al_draw_text(font, al_map_rgb(100, 255, 200),
        SCREEN_W / 2.0f + 30, 160, 0, diff_names[difficulty]);

    // velocidad de nave 
    int speed_idx = (int)(player.speed / 2.0f) - 1;
    if (speed_idx < 0) speed_idx = 0;
    if (speed_idx > 2) speed_idx = 2;
    draw_option(options_labels[2], 190, options_sel == 2);
    al_draw_text(font, al_map_rgb(100, 255, 200),
        SCREEN_W / 2.0f + 30, 190, 0, speed_names[speed_idx]);

    // volver
    draw_option(options_labels[3], 240, options_sel == 3);

    al_draw_text(font, al_map_rgb(80, 80, 100),
        SCREEN_W / 2.0f, SCREEN_H - 30,
        ALLEGRO_ALIGN_CENTER, "Flechas: cambiar  |  Enter/Esc: volver");
}

void update_options(ALLEGRO_EVENT* ev) {
    if (ev->type != ALLEGRO_EVENT_KEY_DOWN) return;

    switch (ev->keyboard.keycode) {
    case ALLEGRO_KEY_UP:
        options_sel = (options_sel - 1 + OPTIONS_COUNT) % OPTIONS_COUNT;
        break;
    case ALLEGRO_KEY_DOWN:
        options_sel = (options_sel + 1) % OPTIONS_COUNT;
        break;
    case ALLEGRO_KEY_LEFT:
        if (options_sel == 0)
            game_mode = (GameMode)((game_mode - 1 + 4) % 4);
        else if (options_sel == 1)
            difficulty = (difficulty - 1 + 8) % 8;
        else if (options_sel == 2) {
            if (player.speed > 2.0f) player.speed -= 2.0f;
        }
        break;
    case ALLEGRO_KEY_RIGHT:
        if (options_sel == 0)
            game_mode = (GameMode)((game_mode + 1) % 4);
        else if (options_sel == 1)
            difficulty = (difficulty + 1) % 8;
        else if (options_sel == 2) {
            if (player.speed < 6.0f) player.speed += 2.0f;
        }
        break;
    case ALLEGRO_KEY_ENTER:
        if (options_sel == 3) game_state = STATE_MENU;
        break;
    case ALLEGRO_KEY_ESCAPE:
        game_state = STATE_MENU;
        break;
    }
}

// ================================
// GAME OVER
// ================================
static int game_over_timer = 0;

void draw_game_over() {
    al_clear_to_color(al_map_rgb(5, 0, 0));

    al_draw_text(font, al_map_rgb(255, 30, 30),
        SCREEN_W / 2.0f, SCREEN_H / 2.0f - 60,
        ALLEGRO_ALIGN_CENTER, "GAME OVER");

    al_draw_textf(font, al_map_rgb(200, 200, 200),
        SCREEN_W / 2.0f, SCREEN_H / 2.0f - 20,
        ALLEGRO_ALIGN_CENTER, "Puntaje: %d", player.score);

    al_draw_textf(font, al_map_rgb(150, 150, 150),
        SCREEN_W / 2.0f, SCREEN_H / 2.0f + 5,
        ALLEGRO_ALIGN_CENTER, "Disparos: %d | Aciertos: %d | Fallos: %d",
        player.shots_fired, player.shots_hit, player.shots_missed);

    if (game_over_timer > 90) {
        al_draw_text(font, al_map_rgb(150, 150, 100),
            SCREEN_W / 2.0f, SCREEN_H / 2.0f + 50,
            ALLEGRO_ALIGN_CENTER, "Enter: ingresar nombre  |  Esc: menu");
    }
}

void update_game_over(ALLEGRO_EVENT* ev) {
    game_over_timer++;
    if (ev->type != ALLEGRO_EVENT_KEY_DOWN) return;

    if (game_over_timer < 60) return;  // evitar skip 

    if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER) {
        game_over_timer = 0;
        game_state = STATE_NAME_ENTRY;
        screens_init();
    }
    if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
        game_over_timer = 0;
        game_state = STATE_MENU;
    }
}

// ================================
// VICTORIA
// ================================
void draw_victory() {
    al_clear_to_color(al_map_rgb(0, 5, 0));

    al_draw_text(font, al_map_rgb(100, 255, 100),
        SCREEN_W / 2.0f, SCREEN_H / 2.0f - 60,
        ALLEGRO_ALIGN_CENTER, "VICTORIA!");

    al_draw_text(font, al_map_rgb(200, 255, 200),
        SCREEN_W / 2.0f, SCREEN_H / 2.0f - 30,
        ALLEGRO_ALIGN_CENTER, "Has derrotado a The Void");

    al_draw_textf(font, al_map_rgb(200, 200, 200),
        SCREEN_W / 2.0f, SCREEN_H / 2.0f,
        ALLEGRO_ALIGN_CENTER, "Puntaje final: %d", player.score);

    al_draw_text(font, al_map_rgb(150, 200, 150),
        SCREEN_W / 2.0f, SCREEN_H / 2.0f + 50,
        ALLEGRO_ALIGN_CENTER, "Enter: guardar score  |  Esc: menu");
}

void update_victory(ALLEGRO_EVENT* ev) {
    if (ev->type != ALLEGRO_EVENT_KEY_DOWN) return;
    if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER) {
        game_state = STATE_NAME_ENTRY;
        screens_init();
    }
    if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
        game_state = STATE_MENU;
    }
}

// ================================
// ESTADISTICAS / TOP SCORES
// ================================
void draw_stats_screen() {
    al_clear_to_color(al_map_rgb(0, 0, 20));
    draw_title("Top 10 Scores", 30);

    // cargar y mostrar el arbol de scores
    load_scores();

    // recorrer el arbol en orden descendente para mostrar en pantalla
    // usamos un array temporal para recolectar los records
    typedef struct { char name[50]; int score; int fired; int hit; int missed; } SR;
    SR records[10];
    int rec_count = 0;

    // funcion lambda emulada con variable estatica — recorre arbol en orden desc
    // usamos una funcion auxiliar de impresion pero guardamos en array
    // (stats.cpp ya tiene print_top_scores; aqui dibujamos en pantalla)
    // Re-implementamos la travesia aqui para obtener los datos en un array

    // --- TRAVESIA IN-ORDER INVERSA DEL ARBOL ---
    // pila manual de hasta 64 nodos para evitar recursion en este contexto
    TreeNode* stack[64];
    int stack_top = 0;
    TreeNode* curr = score_tree;

    while ((curr != NULL || stack_top > 0) && rec_count < 10) {
        // ir al nodo mas a la derecha (mayor puntaje)
        while (curr != NULL) {
            stack[stack_top++] = curr;
            curr = curr->right;
        }
        curr = stack[--stack_top];

        // guardar record
        strncpy(records[rec_count].name, curr->record.name, 49);
        records[rec_count].score = curr->record.score;
        records[rec_count].fired = curr->record.shots_fired;
        records[rec_count].hit = curr->record.shots_hit;
        records[rec_count].missed = curr->record.shots_missed;
        rec_count++;

        curr = curr->left;
    }

    // dibujar la tabla
    float y = 80.0f;
    al_draw_text(font, al_map_rgb(150, 150, 200),
        SCREEN_W / 2.0f, y, ALLEGRO_ALIGN_CENTER,
        "#   Nombre         Score   Aciertos");
    y += 20;
    al_draw_filled_rectangle(20, y, SCREEN_W - 20, y + 1,
        al_map_rgb(60, 60, 100));
    y += 10;

    for (int i = 0; i < rec_count; i++) {
        ALLEGRO_COLOR col = (i == 0)
            ? al_map_rgb(255, 215, 0)    // oro para el 1ro
            : (i == 1)
            ? al_map_rgb(192, 192, 192)  // plata
            : al_map_rgb(200, 200, 200);

        al_draw_textf(font, col,
            30, y + i * 26, 0,
            "%2d. %-16s %6d   %d/%d",
            i + 1,
            records[i].name,
            records[i].score,
            records[i].hit,
            records[i].fired);
    }

    if (rec_count == 0) {
        al_draw_text(font, al_map_rgb(100, 100, 120),
            SCREEN_W / 2.0f, SCREEN_H / 2.0f,
            ALLEGRO_ALIGN_CENTER, "Sin records todavia. Juega una partida!");
    }

    al_draw_text(font, al_map_rgb(80, 80, 100),
        SCREEN_W / 2.0f, SCREEN_H - 30,
        ALLEGRO_ALIGN_CENTER, "Esc: volver al menu");
}

void update_stats_screen(ALLEGRO_EVENT* ev) {
    if (ev->type != ALLEGRO_EVENT_KEY_DOWN) return;
    if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE ||
        ev->keyboard.keycode == ALLEGRO_KEY_ENTER) {
        game_state = STATE_MENU;
    }
}

// ================================
// NAME ENTRY
// El jugador escribe su nombre
// con el teclado; Enter confirma
// ================================
void draw_name_entry() {
    al_clear_to_color(al_map_rgb(0, 0, 25));

    al_draw_text(font, al_map_rgb(200, 200, 255),
        SCREEN_W / 2.0f, SCREEN_H / 2.0f - 60,
        ALLEGRO_ALIGN_CENTER, "Ingresa tu nombre:");

    // cuadro de texto
    float bx = SCREEN_W / 2.0f - 100;
    float by = SCREEN_H / 2.0f - 25;
    al_draw_filled_rectangle(bx, by, bx + 200, by + 30,
        al_map_rgb(10, 10, 50));
    al_draw_rectangle(bx, by, bx + 200, by + 30,
        al_map_rgb(100, 100, 200), 1.5f);

    // texto ingresado con cursor parpadeante
    char display[52];
    snprintf(display, sizeof(display), "%s_", name_entry_buffer);
    al_draw_text(font, al_map_rgb(255, 255, 255),
        SCREEN_W / 2.0f, by + 8, ALLEGRO_ALIGN_CENTER, display);

    al_draw_text(font, al_map_rgb(100, 100, 150),
        SCREEN_W / 2.0f, SCREEN_H / 2.0f + 30,
        ALLEGRO_ALIGN_CENTER, "Enter: confirmar  |  Backspace: borrar");

    al_draw_textf(font, al_map_rgb(80, 80, 100),
        SCREEN_W / 2.0f, SCREEN_H / 2.0f + 55,
        ALLEGRO_ALIGN_CENTER, "Score a guardar: %d", player.score);
}

void update_name_entry(ALLEGRO_EVENT* ev) {
    if (ev->type == ALLEGRO_EVENT_KEY_CHAR) {
        int kc = ev->keyboard.keycode;

        if (kc == ALLEGRO_KEY_ENTER) {
            if (name_entry_len == 0) {
                strncpy(name_entry_buffer, "Anonimo", 49);
                name_entry_len = 7;
            }
            name_entry_done = 1;
            // arrancar partida nueva con el nombre ingresado
            start_new_game();

        }
        else if (kc == ALLEGRO_KEY_BACKSPACE) {
            if (name_entry_len > 0) {
                name_entry_buffer[--name_entry_len] = '\0';
            }
        }
        else if (kc == ALLEGRO_KEY_ESCAPE) {
            game_state = STATE_MENU;

        }
        else if (name_entry_len < 15) {
            // solo caracteres imprimibles
            int ch = ev->keyboard.unichar;
            if (ch >= 32 && ch < 127) {
                name_entry_buffer[name_entry_len++] = (char)ch;
                name_entry_buffer[name_entry_len] = '\0';
            }
        }
    }
}