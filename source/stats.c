/**
 * Grupo 51
 * Henrique Vale 58168
 * Leonardo Monteiro 58250
 * Sergio Esteves 58245
*/

#include "stats.h"
#include <stdlib.h>


struct statistics_t * stats_create() {
    struct statistics_t* stats = malloc(sizeof(struct statistics_t));

    if (stats == NULL) //error in malloc
        return NULL;

    stats->n_clients = 0;
    stats->op_counter = 0;
    stats->time = 0;

    return stats;
}

int stats_increase_ops(struct statistics_t* stats) {
    if (stats == NULL) //error in struct
        return -1;

    stats->op_counter = stats->op_counter + 1;
    return 0;
}

int stats_increase_time(struct statistics_t* stats, long time) {
    if (stats == NULL) //error in struct
        return -1;

    stats->time = stats->time + time;
    return 0;
}

int stats_increase_clients(struct statistics_t* stats) {
    if (stats == NULL) //error in struct
        return -1;

    stats->n_clients = stats->n_clients + 1;
    return 0;
}

int stats_decrease_clients(struct statistics_t* stats) {
    if (stats == NULL) //error in struct
        return -1;
    
    stats->n_clients = stats->n_clients - 1;
    return 0;
}

int stats_destroy(struct statistics_t* stats) {
    if (stats == NULL) //error in struct
        return -1;

    free(stats);
    return 0;
}

struct statistics_t* stats_dup(struct statistics_t* stats) {
    if (stats == NULL)
        return NULL;
    struct statistics_t* copy = malloc(sizeof(struct statistics_t));

    if (copy == NULL)
        return NULL;

    copy->n_clients = stats->n_clients;
    copy->op_counter = stats->op_counter;
    copy->time = stats->time;

    return copy;
}
