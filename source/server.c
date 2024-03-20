/**
 * Grupo 51
 * Henrique Vale 58168
 * Leonardo Monteiro 58250
 * Sergio Esteves 58245
*/

#include "server.h"
#include "server-private.h"
#include <stdlib.h>
#include <string.h>

struct server_t* server_create(int n, char* zookeeper_host, char* addr) {
    struct server_t* server = malloc(sizeof(struct server_t));
    if (server == NULL) //error in malloc
        return NULL;

    pthread_mutex_init(&server->table_lock, NULL);

    if (&server->table_lock == NULL) { //error in mutex init
        free(server);
        return NULL;
    }
    
    pthread_mutex_init(&server->stats_lock, NULL);

    if (&server->stats_lock == NULL) {
        pthread_mutex_destroy(&server->table_lock);
        free(server);
        return NULL;
    }


    server->stats = stats_create();

    if (server->stats == NULL) { //error creating stats
        pthread_mutex_destroy(&server->table_lock);
        pthread_mutex_destroy(&server->stats_lock);
        free(server);
        return NULL;
    }

    server->table = table_create(n);

    if (server->table == NULL) { //error creating table
        pthread_mutex_destroy(&server->table_lock);
        pthread_mutex_destroy(&server->stats_lock);
        stats_destroy(server->stats);
        free(server);
        return NULL;
    }

    server->zookeper = zookeeper_init(zookeeper_host,connection_watcher,2000,0,NULL,0);
    
    while(!is_connected) {}

    if (server->zookeper == NULL) {
        pthread_mutex_destroy(&server->table_lock);
        pthread_mutex_destroy(&server->stats_lock);
        stats_destroy(server->stats);
        free(server->table);
        free(server);  
        return NULL;
    }

    if (ZNONODE == zoo_exists(server->zookeper, "/chain",0,NULL)) {
        zoo_create(server->zookeper,"/chain",NULL,-1,& ZOO_OPEN_ACL_UNSAFE,0,NULL,0);
    }

    server->node = malloc (1024);

    if (server->node == NULL) {
        pthread_mutex_destroy(&server->table_lock);
        pthread_mutex_destroy(&server->stats_lock);
        stats_destroy(server->stats);
        free(server->table);
        free(server);  
        return NULL;
    }

    server->node = malloc (ZDATALEN);

    if (zoo_create(server->zookeper, "/chain/node", addr, strlen(addr) * sizeof(char), & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, server->node, ZDATALEN) != ZOK) {
        fprintf(stderr, "Error creating znode from path !\n");
    }


    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));

    int retval = zoo_wget_children(server->zookeper, "/chain", child_watcher, server,children_list);


	if (retval != ZOK)	{
		fprintf(stderr, "Error retrieving children from path /chain!\n");
	    exit(EXIT_FAILURE);
	}

    server->next_id = NULL;
    server->next_server = NULL;

    order_node_children(children_list);
    assign_next_server(server, children_list);
    get_data_from_previous(server, children_list);

    free(children_list);
    return server;
}

int server_put(struct server_t* server, char* key, struct data_t* data) {
    if (server == NULL)
        return -1;

    pthread_mutex_lock(&server->table_lock);
    int result = table_put(server->table, key, data);
    pthread_mutex_unlock(&server->table_lock);

    if (server->next_id != NULL) {
        struct entry_t* e = entry_create(strdup(key), data_dup(data));
        rtable_put(server->next_server, e);
    }
    return result;
}

struct data_t* server_get(struct server_t* server, char* key) {
    if (server == NULL)
        return NULL;
    return table_get(server->table, key);
}

int server_delete(struct server_t* server, char* key) {
    if (server == NULL)
        return -1;

    pthread_mutex_lock(&server->table_lock);
    int result = table_remove(server->table, key);
    pthread_mutex_unlock(&server->table_lock);
    if (server->next_id != NULL) {
        rtable_del(server->next_server, key);
    }
    return (result == 0) ? 0 : -1;
}

int server_size(struct server_t* server) {
    if (server == NULL)
        return -1;
    return table_size(server->table);
}


char** server_keys(struct server_t* server) {
    if (server == NULL)
        return NULL;
    return table_get_keys(server->table);
}


int server_free_keys(char** keys) {
    if (keys == NULL)
        return -1;
    
    return table_free_keys(keys);
}


struct statistics_t* server_stats(struct server_t* server) {
    if (server == NULL)
        return NULL;
    return stats_dup(server->stats);
}

int server_stats_destroy(struct statistics_t* stats) {
    if (stats == NULL)
        return -1;
    return stats_destroy(stats);
}

int server_increase_ops(struct server_t* server) {
    if (server == NULL)
        return -1;
    pthread_mutex_lock(&server->stats_lock);
    int result = stats_increase_ops(server->stats);
    pthread_mutex_unlock(&server->stats_lock);

    return result;
}

int server_increase_time(struct server_t* server, long time) {
    if (server == NULL)
        return -1;

    pthread_mutex_lock(&server->stats_lock);
    int result = stats_increase_time(server->stats, time);
    pthread_mutex_unlock(&server->stats_lock);

    return result;
}


int server_destroy(struct server_t* server) {
    if (server == NULL)
        return -1;
    if (pthread_mutex_destroy(&server->table_lock) != 0)
        return -1;

    if (pthread_mutex_destroy(&server->stats_lock) != 0)
        return -1;

    if (stats_destroy(server->stats) == -1)
        return -1;

    if (table_destroy(server->table) == -1)
        return -1;
    rtable_disconnect(server->next_server);
    free(server->next_id);
    free(server->node);
    zookeeper_close(server->zookeper);
    free(server);

    return 0;
}

int server_increase_clients(struct server_t* server) {
    if (server == NULL)
        return -1;
    
    server->stats->n_clients += 1;
    return 0;
}

int server_decrease_clients(struct server_t* server) {
    if (server == NULL)
        return -1;
    server->stats->n_clients -= 1;
    return 0;
}



void order_node_children(zoo_string* children_list) {
    char *temp = malloc(1024);
    for (int i = 0; i < children_list->count; i++)  {
        for (int j = i +1; j < children_list->count; j++) {
            if (strcmp(children_list->data[i], children_list->data[j]) > 0) {
                strcpy(temp, children_list->data[i]);
                strcpy(children_list->data[i], children_list->data[j]);
                strcpy(children_list->data[j], temp);
			}
        }
    }

    free(temp);
}

void assign_next_server(struct server_t* server, zoo_string* children_list) {
    int len = ZDATALEN;
    printf("Looking for next higher node to %s...\n", server->node);

    for (int i = 0; i < children_list->count; i++)  {
        if (strcmp(children_list->data[i], server->node + 7 * sizeof(char)) == 0) {

            if (i + 1 < children_list->count) {


                server->next_id = malloc(1024);
                snprintf(server->next_id, 1024,"%s/%s",root, children_list->data[i+1]);

                char* buf = malloc(ZDATALEN);
                if (zoo_get(server->zookeper,server->next_id,0, buf, &len,NULL) != ZOK) {
                    printf("Error getting data from %s\n", server->next_id);
                }

                server->next_server = rtable_connect(buf);

                if(server->next_server == NULL) {
                    printf("Error connecting to next_server %s\n", server->next_id);
                }
                

                else printf("Next_server assigned to %s\n", server->next_id);

                free(buf);

                return;
            }
        }
    }
    printf("No next higher node found.\n");
}

void get_data_from_previous(struct server_t* server, zoo_string* children_list) {
    int len = ZDATALEN;
    char* buf = NULL;

    printf("Getting data from previous server...\n");
    for (int i = 0; i < children_list->count; i++)  {
        
        if (strcmp(children_list->data[i], server->node + 7 * sizeof(char)) == 0) {
            
            if (i - 1 >= 0) {
        
                char path [1024];
                buf = malloc(ZDATALEN);
                snprintf(path,1024,"%s/%s", "/chain", children_list->data[i - 1]);
        
                if (zoo_get(server->zookeper,path,0, buf, &len,0) != ZOK) {
                    printf("Error getting data from %s\n", path);
                    return;
                }

                struct rtable_t* temp = rtable_connect(buf);
                printf("Copying data from %s...\n", path);
        
                copy_table(server, temp);
                printf("Data copied.\n");
                free(buf);
                rtable_disconnect(temp);

                return;
            }
        } 
    }

    printf("No server found.\n");
}

void copy_table(struct server_t* server, struct rtable_t* other) {
    struct entry_t** entries = rtable_get_table(other);
    if (entries == NULL) return;
    int i = 0;
    while (entries[i] != NULL) {
        table_put(server->table, entries[i]->key, entries[i]->value);
        i++;
    }

    rtable_free_entries(entries);
}


static void child_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcher_ctx) {
    printf("Change in /chain's children detected\n");
    struct server_t* server = (struct server_t*) watcher_ctx;
    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));

    

    zoo_wget_children(zh,"/chain",child_watcher, watcher_ctx, children_list);

    order_node_children(children_list);

    if (server->next_id == NULL || zoo_exists(zh, server->next_id,0,NULL)==ZNONODE) {

        free(server->next_id);

        server->next_id = NULL;

        rtable_disconnect(server->next_server);

        server->next_server = NULL;

        assign_next_server(server, children_list);
    }
}


void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1; 
		} else {
			is_connected = 0; 
		}
	} 
}
