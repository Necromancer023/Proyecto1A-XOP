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

// nombre del archivo de historial
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
// recorre en orden descendente (derecha primero)
// ================================
void print_top_scores(TreeNode* root, int* count, int max) {
    if (root == NULL || *count >= max) return;

    // derecha primero = mayor puntaje primero
    print_top_scores(root->right, count, max);

    if (*count < max) {
        (*count)++;
        printf("%d. %s — %d pts | Disparos: %d | Aciertos: %d | Fallos: %d\n",
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

    fprintf(f, "%s %d %d %d %d\n",
        record.name,
        record.score,
        record.shots_fired,
        record.shots_hit,
        record.shots_missed);

    fclose(f);
}

// ================================
// LEER ARCHIVO Y CARGAR AL ARBOL
// ================================
void load_scores() {
    free_tree(score_tree);
    score_tree = NULL;

    FILE* f = fopen(SCORES_FILE, "r");
    if (f == NULL) return;  // archivo no existe todavia

    GameRecord record;
    while (fscanf(f, "%s %d %d %d %d",
        record.name,
        &record.score,
        &record.shots_fired,
        &record.shots_hit,
        &record.shots_missed) == 5) {

        score_tree = insert_node(score_tree, record);
    }

    fclose(f);
}

// ================================
// GUARDAR PARTIDA ACTUAL DEL JUGADOR
// ================================
void show_top_scores() {
    load_scores();
    int count = 0;
    printf("\n===== TOP 10 SCORES =====\n");
    print_top_scores(score_tree, &count, 10);
    printf("=========================\n\n");
}

// ================================
// CONSTRUIR RECORD DE LA PARTIDA ACTUAL
// ================================
void save_current_game(const char* name) {
    GameRecord record;
    strncpy(record.name, name, 49);
    record.score = player.score;
    record.shots_fired = player.shots_fired;
    record.shots_hit = player.shots_hit;
    record.shots_missed = player.shots_missed;

    save_record(record);
}