/**
 * Grupo 51
 * Henrique Vale 58168
 * Leonardo Monteiro 58250
 * Sergio Esteves 58245
*/

#ifndef _SERVER_H
#define _SERVER_H

#include "stats.h"
#include "table.h"
#include "client_stub.h"
#include <pthread.h>
#include <zookeeper/zookeeper.h>



struct server_t; //defined in server-private.h

struct thread_args {
    int connsockfd;
    struct server_t* server;
};

/**
 * Creates a struct for a server with a table with n lines and stats. Returns NULL if an error occurs
*/
struct server_t* server_create(int n, char* zookeeper_host, char* addr);

/**
 * Puts a <key, value> pair in a table, stopping other threads from accessing it during the process. 
 * Returns 0 if successful, or -1 otherwise
*/
int server_put(struct server_t* server, char* key, struct data_t* data);

/**
 * Returns the value paired to the key from the table, or NULL if an error occurss
*/
struct data_t* server_get(struct server_t* server, char* key);

/**
 * Removes a <key,value> pair from a table, stopping other threads from accessing it during the process and changes the amount of ops performed and time taken. 
 * Returns 0 if successful, or -1 otherwise
*/
int server_delete(struct server_t* server, char* key);

/**
 * Returns an array with the keys from the server table 
*/
char** server_keys(struct server_t* server);

/**
 * Free a given keys array
*/
int server_free_keys(char** keys);

/**
 * Returns an array with the entries from the server table
*/
struct entry_t* server_table(struct server_t* server);

/**
 * Returns the server size or -1 if an error occured
*/
int server_size(struct server_t* server);

/**
 * Returns a copy of the server stats or NULL in case of error
*/
struct statistics_t* server_stats(struct server_t* server);

/**
 * Frees up memory occupied by the server statistics. Returns 0 if successful or -1 otherwise
*/
int server_stats_destroy(struct statistics_t* stats);


/**
 * Increases the counter for the amount of operations performed by one. Prevents access from other threads
 * Returns 0 if successful, or -1 otherwise.
*/
int server_increase_ops(struct server_t* server);

/**
 * Increases the time spent performing operations by a given amount. Prevents access from other threads.
 * Returns 0 if successful or -1 otherwise
*/
int server_increase_time(struct server_t* server, long time);

/**
 * Increases the counter for the amount of clients connected to the server
*/
int server_increase_clients(struct server_t* server);

/**
 * Decreases the counter for the amount fo clients connected to the server. Returns 0 if successful or -1 otherwise
*/
int server_decrease_clients(struct server_t* server);

/**
 * Frees up memory occupied by the server. Returns 0 if successful or -1 in case of error. Returns 0 if successful or -1 otherwise
*/
int server_destroy(struct server_t* server);



#endif