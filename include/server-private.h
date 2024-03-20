#ifndef _SERVER_PRIVATE_H
#define _SERVER_PRIVATE_H

#include "server.h"

struct server_t
{
    struct table_t *table;
    struct statistics_t* stats;
    pthread_mutex_t table_lock;
    pthread_mutex_t stats_lock;
    zhandle_t* zookeper;
    char* node;
    char* next_id;
    struct rtable_t* next_server;
};

typedef struct String_vector zoo_string;

#define ZDATALEN 1024 * 1024
int is_connected;
static char * root = "/chain";


/**
 * Assigns a successor to the current server from a string of zookeeper children nodes
*/
void assign_next_server(struct server_t* server, zoo_string* children_list);

/**
 * Orders a string of zookeeper children nodes
*/
void order_node_children(zoo_string* children_list);

/**
 * Watcher function for zookeeper children
*/
static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx);

/**
 * Watcher function for estabilishing connection
*/
void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context);


/**
 * Function for getting data from a predecessor server
*/
void get_data_from_previous(struct server_t* server, zoo_string* children_list);

/**
 * Copies a table from a nother server to the current server
*/
void copy_table(struct server_t* server, struct rtable_t* other);



#endif