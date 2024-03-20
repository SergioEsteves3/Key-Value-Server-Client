/**
 * Grupo 51
 * Henrique Vale 58168
 * Leonardo Monteiro 58250
 * Sergio Esteves 58245
*/

#ifndef _STATS_H_
#define _STATS_H_

#include <sys/time.h>

struct statistics_t
{
    int op_counter;
    long time; //time in microseconds
    int n_clients;
};

/**
 * Creates a structure for storing statitics, with all values at 0. Returns Null if an error occurs
*/
struct statistics_t * stats_create();

/**
 * Increases the operation counter by one. Returns 0 if successful or -1 if an error occurs
*/
int stats_increase_ops(struct statistics_t* stats);

/**
 * Increases the time spent by a given amount, in microseconds. Returns 0 if successful or -1 if an error occurs
*/
int stats_increase_time(struct statistics_t* stats, long time);

/**
 * Increases the amount of clients connected by one. Returns 0 if successful or -1 if an error occurs
*/
int stats_increase_clients(struct statistics_t* stats);

/**
 * Decreases the amount of clients by one. Returns 0 if successful or -1 if an error occurs
*/
int stats_decrease_clients(struct statistics_t* stats);

/**
 * Frees up memory allocated to the stats structure. Returns 0 if successful and -1 if an error occurred
*/
int stats_destroy(struct statistics_t* stats);

/**
 * Returns a copy of a given stats structure or NULL in case of error
*/
struct statistics_t* stats_dup(struct statistics_t* stats);



#endif