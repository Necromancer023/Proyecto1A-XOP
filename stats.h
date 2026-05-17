#ifndef STATS_H
#define STATS_H

#include "game.h"

// ================================
// STRUCT PARA UNA PARTIDA
// ================================
typedef struct {
    char name[50];
    int  score;
    int  shots_fired;
    int  shots_hit;
    int  shots_missed;
} GameRecord;

// ================================
// NODO DEL ARBOL BINARIO
// ================================
typedef struct TreeNode {
    GameRecord      record;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

// ================================
// VARIABLE GLOBAL DEL ARBOL
// ================================
extern TreeNode* score_tree;

// ================================
// FUNCIONES DEL ARBOL
// ================================
TreeNode* create_node(GameRecord record);
TreeNode* insert_node(TreeNode* root, GameRecord record);
void      print_top_scores(TreeNode* root, int* count, int max);
void      free_tree(TreeNode* root);

// ================================
// FUNCIONES DE ARCHIVO
// ================================
void save_record(GameRecord record);
void load_scores();
void show_top_scores();
void save_current_game(const char* name);

#endif