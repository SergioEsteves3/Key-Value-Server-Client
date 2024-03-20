/**
 * Grupo 51
 * Henrique Vale 58168
 * Leonardo Monteiro 58250
 * Sergio Esteves 58245
*/

#include "client_stub.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <zookeeper/zookeeper.h>

struct rtable_t * head;
char* head_id;
struct rtable_t * tail;
char* tail_id;
static zhandle_t* zh;
static int is_connected = 0;
static char *watcher_ctx = "ZooKeeper Data Watcher";
#define ZDATALEN 1024 * 1024



char* root = "/chain";

typedef struct String_vector zoo_string;


void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1; 
		} else {
			is_connected = 0; 
		}
	}
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


static void child_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcher_ctx) {
    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
    zoo_wget_children(zh,"/chain",child_watcher, watcher_ctx, children_list);
    order_node_children(children_list);

    if (strcmp(children_list->data[0], head_id + 7 * sizeof(char)) != 0) {

        snprintf(head_id,1024,"%s/%s",  root, children_list->data[0]);
        printf("Changing head to %s\n", head_id);
        if (rtable_disconnect(head) == -1) {
            printf("Error disconnecting from previous head\n");
        }

        char* buffer = malloc (ZDATALEN);
        int len = ZDATALEN;
        zoo_get(zh, head_id,0,buffer, &len ,NULL);

        if ((head = rtable_connect(buffer)) == NULL) {
            printf("Error connecting to head\n");
        }
        free(buffer);

    }
    if (strcmp(children_list->data[(children_list->count) - 1], tail_id + 7 * sizeof(char)) != 0) {
        snprintf(tail_id,1024,"%s/%s",  root, children_list->data[children_list->count-1]);
        printf("Changing tail to %s\n", tail_id);
        if (rtable_disconnect(tail) == -1) {
            printf("Error disconnecting from previous tail\n");
        }

            char* buffer = malloc (ZDATALEN);
            int len = ZDATALEN;
            zoo_get(zh, tail_id,0,buffer, &len ,NULL);

        if ((tail = rtable_connect(buffer)) == NULL) {
            printf("Error connecting to tail\n");
        }
        free(buffer);
    }
}

char* data_to_string(const struct data_t *data) {
    if (data != NULL && data->data != NULL && data->datasize > 0) {
        char *charData = (char *)data->data;
        char *str = (char *)malloc(data->datasize + 1);

        if (str != NULL) {
            strncpy(str, charData, data->datasize);
            str[data->datasize] = '\0';
            return str;
        }
    }

    return NULL;
}

void handle_server_close() {
    //do nothing
}

void quit() {
    if (rtable_disconnect(head) == -1 || (strcmp(head_id, tail_id) != 0 && rtable_disconnect(tail)) || zookeeper_close(zh)) {
        perror("Error disconnecting from server");
        exit(-1);
    }

    printf("Disconnected from server\n");
    exit(0);
}

int main(int argc, char **argv) {
    if (argc != 2){ //incorrect args
        printf("Use: ./table_client <zookeeper_ip>:<zookeeper_port>\n");
        printf("Example: ./table_client 127.0.0.1:2281\n");
        return -1;
    }


    if((zh = zookeeper_init(argv[1], connection_watcher,2000,0,0,0)) == NULL) {
        printf("Error connecting to Zookeeper\n");
    }
    while (is_connected==0){}
    

    if(zoo_exists(zh, "/chain",0,NULL)) printf("SUCCESSS\n\n");

    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));

    int retval = zoo_wget_children(zh, "/chain", child_watcher,watcher_ctx,children_list);
    if (retval != ZOK) {
        printf("Error getting children:%d\n", retval);
        return -1;
    }
    order_node_children(children_list);

    head_id = malloc(1024);
    snprintf(head_id,1024,"%s/%s",  root, children_list->data[0]);

    tail_id = malloc(1024);
    snprintf(tail_id, 1024, "%s/%s", root,children_list->data[children_list->count - 1]);




    printf("Setting %s as head\n", head_id);
    char*buffer = malloc(ZDATALEN);
    int len = ZDATALEN;

    zoo_get(zh, head_id,0,buffer, &len ,NULL);

    if ((head = rtable_connect(buffer)) == NULL) {
        printf("Error connecting to head\n");
    }
    free(buffer);

    printf("Setting %s as tail\n", tail_id);


    buffer = malloc(ZDATALEN);
    len = ZDATALEN;

    zoo_get(zh, tail_id,0,buffer, &len ,NULL);

    if ((tail = rtable_connect(buffer)) == NULL) {
    printf("Error connecting to tail\n");

    }
    free(buffer);





    char input[512];
    
    //handle relevant signals
    signal(SIGPIPE, handle_server_close);
    signal(SIGINT, quit);

    while (1) {

        printf("Command:");

		fgets(input, 512, stdin); //get command
        input[strcspn(input, "\n")] = '\0'; //remove newline and add terminator


        if (input[0] == '\0') continue; //no input


        if (strcmp(input, "quit") == 0 || strcmp(input, "q") == 0) {
            quit();
        }

        char* request;
        request = strtok(input," "); //get command
	
        if (strcmp(request, "put") == 0 || strcmp(request, "p") == 0) {
            char* key = strtok(NULL, " "); //get key
            char* token = strtok(NULL, " "); //get first data token


            if (key == NULL || token == NULL) { // no key or data
                printf("Invalid arguments. Usage: put <key> <value>\n");
                continue;
            }


            char value[255];
            value[0] = '\0';
            int len = 0;
            do
            {
                strcat(value, token);
                len += strlen(token);
                token = strtok(NULL, " ");

                if(token != NULL) {
                    strcat(value, " ");
                    len += 1;
                }

            } while (token != NULL);


            struct data_t* data = data_create(len, value);

            struct entry_t *entry = entry_create(key,data);


            if(rtable_put(head, entry) == -1) {
                printf("Error putting entry into table\n");
                continue;

            }

        }

        else if (strcmp(request, "get") == 0 || strcmp(request, "g") == 0) {
            char* key = strtok(NULL, " "); //get key

            if (key == NULL) {
                printf("Invalid arguments. Usage: get <key>\n");
                continue;
            }

            struct data_t *data = rtable_get(tail, key);
            if(data == NULL) {
                printf ("Error getting data from server or unable to find key\n");
                continue;
            }

            //print data
            char* data_string = data_to_string(data);
            printf("%s\n", data_string);

            free(data_string);
            data_destroy(data);
        }

        else if (strcmp(request, "size") == 0 || strcmp(request, "s") == 0) {
            
            int size = rtable_size(tail);

            if (size == -1) {
                printf("Error getting size from server\n");
                continue;
            }

            printf("Table size: %d\n", size);
        }

        else if (strcmp(request, "del") == 0 || strcmp(request, "d") == 0) {
            char* key = strtok(NULL, " ");
            if (key == NULL) {
                printf("Invalid arguments. Usage: del <key>\n");
                continue;
            }

            if (rtable_del(head, key) == -1) {
                printf ("Error deleting data from server or unable to find key\n");
                continue;
            }

            printf("Entry removed\n");
        }

        else if (strcmp (request, "getkeys") == 0 ||strcmp(request, "k") == 0) {

            char** keys = rtable_get_keys(tail);

            if(keys == NULL) {
                printf("Error getting keys from server or no keys available\n");
                continue;
            }

            int i = 0;

            //print keys
            while(keys[i] != NULL) {

                printf("%s\n",keys[i]);
                i++;
            }

            rtable_free_keys(keys);
        }

        else if (strcmp(request, "gettable") == 0 || strcmp(request, "t") == 0) {
            struct entry_t** entries = rtable_get_table(tail);
            
            if(entries == NULL) {
                printf("Error getting table\n");
                continue;
            }
            int i = 0;

            //print entries
            while (entries[i] != NULL) {
                char* data = data_to_string(entries[i]->value);
                printf("%s :: %s", entries[i]->key, data);
                printf("\n");
                i++;
                free(data);
            }

            rtable_free_entries(entries);
        }

        else if (strcmp(request, "stats") == 0 || strcmp(request, "st") == 0) {
            struct statistics_t* stats = rtable_stats(tail);
            if (stats == NULL) {
                printf("Error getting server statistics\n");
                continue;
            }

            printf("Clients connected: %d\n", stats->n_clients);
            printf("Operations performed: %d\n", stats->op_counter);
            printf("Time elapsed: %ld us\n", stats->time);

            stats_destroy(stats);
        }

        

        else { //unrecognized command
            printf("Unrecognized command.\nAvailable commands:\n");
                        printf("     p[ut] <key> <data>\n");
                        printf("     g[et] <key>\n");
                        printf("     s[ize]\n");
                        printf("     d[el] <key>\n");
                        printf("     [get]k[eys]\n");
                        printf("     [get]t[able]\n");
                        printf("     st[ats]\n");
                        printf("     q[uit]\n");
        }

    }


    
}



