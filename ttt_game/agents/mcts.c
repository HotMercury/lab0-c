#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../game.h"
#include "mcts.h"
#include "util.h"
#define FIX_POINT
struct node {
    int move;
    char player;
    int n_visits;
    double score;
    unsigned long score_fix;
    struct node *parent;
    struct node *children[N_GRIDS];
};

static struct node *new_node(int move, char player, struct node *parent)
{
    struct node *node = malloc(sizeof(struct node));
    node->move = move;
    node->player = player;
    node->n_visits = 0;
    node->score = 0;
    node->score_fix = 0;
    node->parent = parent;
    memset(node->children, 0, sizeof(node->children));
    return node;
}

static void free_node(struct node *node)
{
    for (int i = 0; i < N_GRIDS; i++)
        if (node->children[i])
            free_node(node->children[i]);
    free(node);
}


// fx48.16 sqrt
static inline unsigned long sqrt_fix(unsigned long a, int fix_point)
{
    unsigned long x = 1 << fix_point;
    unsigned long n_one = a << fix_point;
    unsigned long x_old;
    while (1) {
        x_old = x;
        x = (x + n_one / x) >> 1;
        if (x >= x_old)
            break;
    }
    return x;
}
static inline unsigned long log_fix(unsigned long a, int fix_point)
{
    // Check if a is non-positive
    if (a <= 0) {
        return 0;  // Return 0 or some other error handling mechanism
    }

    // Convert to floating point for logarithm calculation
    double log_value = log((double) a);

    // Scale the logarithm value to the fixed point representation
    unsigned long result = (unsigned long) (log_value * (1 << fix_point));

    return result;
}
static inline unsigned long multiply_fix(unsigned long a,
                                         unsigned long b,
                                         int fix_point)
{
    return (a * b) >> fix_point;
}
// fx48.16
static inline unsigned long divide_fix(unsigned long a,
                                       unsigned long b,
                                       int fix_point)
{
    unsigned long quotient = 0;
    a = a << fix_point;
    // Perform division
    if (b != 0) {
        quotient = a / b;
    }
    return quotient;
}
static inline double uct_score(int n_total, int n_visits, double score)
{
    if (n_visits == 0)
        return DBL_MAX;
    return score / n_visits +
           EXPLORATION_FACTOR * sqrt(log(n_total) / n_visits);
}
static inline unsigned long uct_score_fix(int n_total,
                                          int n_visits,
                                          unsigned long score,
                                          int fix_point)
{
    if (n_visits == 0)
        return 1UL << 63;
    return divide_fix(score, n_visits, fix_point) +
           multiply_fix(sqrt_fix(2, 16),
                        sqrt_fix(divide_fix(log_fix(n_total, fix_point),
                                            n_visits, fix_point),
                                 fix_point),
                        fix_point);
}
// using fix point to calculate uct score
static struct node *select_move_fix(struct node *node)
{
    struct node *best_node = NULL;

    for (int i = 0; i < N_GRIDS; i++) {
        if (!node->children[i])
            continue;
#ifdef FIX_POINT
        unsigned long best_score = 0;
        unsigned long score =
            uct_score_fix(node->n_visits, node->children[i]->n_visits,
                          node->children[i]->score, 16);
#else
        double best_score = -1;
        double score = uct_score(node->n_visits, node->children[i]->n_visits,
                                 node->children[i]->score);
#endif
        if (score > best_score) {
            best_score = score;
            best_node = node->children[i];
        }
    }
    return best_node;
}

static struct node *select_move(struct node *node)
{
    struct node *best_node = NULL;
    double best_score = -1;
    for (int i = 0; i < N_GRIDS; i++) {
        if (!node->children[i])
            continue;
        double score = uct_score(node->n_visits, node->children[i]->n_visits,
                                 node->children[i]->score);
        if (score > best_score) {
            best_score = score;
            best_node = node->children[i];
        }
    }
    return best_node;
}

// random simulate and return probabilty of win
static double simulate(char *table, char player)
{
    char win;
    char current_player = player;
    char temp_table[N_GRIDS];
    memcpy(temp_table, table, N_GRIDS);
    while (1) {
        int *moves = available_moves(temp_table);
        if (moves[0] == -1) {
            free(moves);
            break;
        }
        int n_moves = 0;
        while (n_moves < N_GRIDS && moves[n_moves] != -1)
            ++n_moves;
        int move = moves[rand() % n_moves];
        free(moves);
        temp_table[move] = current_player;
        if ((win = check_win(temp_table)) != ' ')
            return calculate_win_value(win, player);
        current_player ^= 'O' ^ 'X';
    }
    return 0.5;
}
static unsigned long simulate_fix(char *table, char player)
{
    char win;
    char current_player = player;
    char temp_table[N_GRIDS];
    memcpy(temp_table, table, N_GRIDS);
    while (1) {
        int *moves = available_moves(temp_table);
        if (moves[0] == -1) {
            free(moves);
            break;
        }
        int n_moves = 0;
        while (n_moves < N_GRIDS && moves[n_moves] != -1)
            ++n_moves;
        int move = moves[rand() % n_moves];
        free(moves);
        temp_table[move] = current_player;
        if ((win = check_win(temp_table)) != ' ')
            return calculate_win_value(win, player);
        current_player ^= 'O' ^ 'X';
    }
    return 1 << 15;
}

/*
1 - score represent player X or O
*/
static void backpropagate(struct node *node, double score)
{
    while (node) {
        node->n_visits++;
        node->score += score;
        node = node->parent;
        score = 1 - score;
    }
}
static void backpropagate_fix(struct node *node, unsigned long score)
{
    while (node) {
        node->n_visits++;
        node->score_fix += score;
        node = node->parent;
        score = (1 << 16) - score;
    }
}
// find all move and new number of moves childrens
static void expand(struct node *node, char *table)
{
    int *moves = available_moves(table);
    int n_moves = 0;
    while (n_moves < N_GRIDS && moves[n_moves] != -1)
        ++n_moves;
    for (int i = 0; i < n_moves; i++) {
        node->children[i] = new_node(moves[i], node->player ^ 'O' ^ 'X', node);
    }
    free(moves);
}

int mcts(char *table, char player)
{
    char win;
    struct node *root = new_node(-1, player, NULL);
    for (int i = 0; i < ITERATIONS; i++) {
        struct node *node = root;
        char temp_table[N_GRIDS];
        memcpy(temp_table, table, N_GRIDS);
        while (1) {
            if ((win = check_win(temp_table)) != ' ') {
#ifdef FIX_POINT
                unsigned long score =
                    calculate_win_value_fix(win, node->player ^ 'O' ^ 'X');
                backpropagate_fix(node, score);
#else
                double score =
                    calculate_win_value(win, node->player ^ 'O' ^ 'X');
                backpropagate(node, score);
#endif

                break;
            }
            if (node->n_visits == 0) {
#ifdef FIX_POINT
                unsigned long score_fix = simulate(temp_table, node->player);
                backpropagate_fix(node, score_fix);
#else
                double score = simulate(temp_table, node->player);
                backpropagate(node, score);
#endif
                break;
            }
            if (node->children[0] == NULL)
                expand(node, temp_table);
// node = select_move(node);
#ifdef FIX_POINT
            node = select_move_fix(node);
#else
            node = select_move(node);
#endif
            // assert(node);
            temp_table[node->move] = node->player ^ 'O' ^ 'X';
        }
    }
    struct node *best_node = NULL;
    int most_visits = -1;
    for (int i = 0; i < N_GRIDS; i++) {
        if (root->children[i] && root->children[i]->n_visits > most_visits) {
            most_visits = root->children[i]->n_visits;
            best_node = root->children[i];
        }
    }
    int best_move = best_node->move;
    free_node(root);
    return best_move;
}
