#define _CRT_SECURE_NO_WARNINGS
#include "stats.h"
#include "player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ================================
// VARIABLE GLOBAL
// ================================
TreeNode* score_tree = NULL;

#define SCORES_FILE "scores.txt"

// ================================
// CREAR NODO DEL ARBOL
// ================================
TreeNode* create_node(GameRecord record) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if (node == NULL) return NULL;
    node->record = record;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// ================================
// INSERTAR EN ARBOL BINARIO
// mayor puntaje va a la derecha
// ================================
TreeNode* insert_node(TreeNode* root, GameRecord record) {
    if (root == NULL) {
        return create_node(record);
    }

    if (record.score < root->record.score) {
        root->left = insert_node(root->left, record);
    }
    else {
        root->right = insert_node(root->right, record);
    }

    return root;
}

// ================================
// MOSTRAR TOP SCORES
// ================================
void print_top_scores(TreeNode* root, int* count, int max) {
    if (root == NULL || *count >= max) return;

    print_top_scores(root->right, count, max);

    if (*count < max) {
        (*count)++;
        printf("%d. %s - %d pts | Disparos: %d | Aciertos: %d | Fallos: %d\n",
            *count,
            root->record.name,
            root->record.score,
            root->record.shots_fired,
            root->record.shots_hit,
            root->record.shots_missed);
    }

    print_top_scores(root->left, count, max);
}

// ================================
// LIBERAR MEMORIA DEL ARBOL
// ================================
void free_tree(TreeNode* root) {
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

// ================================
// GUARDAR PARTIDA EN ARCHIVO
// ================================
void save_record(GameRecord record) {
    FILE* f = fopen(SCORES_FILE, "a");
    if (f == NULL) return;
    
    char safe_name[50];
    strncpy(safe_name, record.name, 49);
    safe_name[49] = '\0';
    for (int i = 0; safe_name[i]; i++)
        if (safe_name[i] == ' ') safe_name[i] = '_';
    fprintf(f, "%s %d %d %d %d\n",
        safe_name, record.score, record.shots_fired,
        record.shots_hit, record.shots_missed);
    fflush(f);
    fclose(f);
}

// ================================
// LEER ARCHIVO Y CARGAR AL ARBOL
// ================================
void load_scores() {
    free_tree(score_tree);
    score_tree = NULL;

    FILE* f = fopen(SCORES_FILE, "r");
    if (f == NULL) return;

    GameRecord record;
    while (fscanf(f, "%49s %d %d %d %d",
        record.name,
        &record.score,
        &record.shots_fired,
        &record.shots_hit,
        &record.shots_missed) == 5) {

        // filtrar entradas "En progreso" del historico visible
        if (strcmp(record.name, "En_progreso") == 0) continue;
        if (strcmp(record.name, "Abandonado") == 0) continue;
        if (strcmp(record.name, "Jugador") == 0) continue;
        // restaurar guiones bajos a espacios para mostrar en pantalla
        for (int i = 0; record.name[i]; i++)
            if (record.name[i] == '_') record.name[i] = ' ';

        score_tree = insert_node(score_tree, record);
    }

    fclose(f);
}

// ================================
// MOSTRAR TOP 10
// ================================
void show_top_scores() {
    load_scores();
    int count = 0;
    printf("\n===== TOP 10 SCORES =====\n");
    print_top_scores(score_tree, &count, 10);
    printf("=========================\n\n");
}

// ================================
// GUARDAR PARTIDA ACTUAL
// shots_missed = disparos que salieron de pantalla sin impactar
// ================================
void save_current_game(const char* name) {
    GameRecord record;
    strncpy(record.name, name, 49);
    record.name[49] = '\0';
    record.score = player.score;
    record.shots_fired = player.shots_fired;
    record.shots_hit = player.shots_hit;
    // shots_missed = disparos lanzados menos los que acertaron
    // incluye los que salieron de pantalla y los que el ReflectShield convirtio en balas propias sin acertar
    record.shots_missed = player.shots_missed;

    save_record(record);
}