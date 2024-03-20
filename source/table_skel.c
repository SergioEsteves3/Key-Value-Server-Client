/**
 * Grupo 51
 * Henrique Vale 58168
 * Leonardo Monteiro 58250
 * Sergio Esteves 58245
*/

#include "table_skel.h"
#include <sys/time.h>
#include "string.h"
#include <stdlib.h>

struct server_t *table_skel_init(int n_lists, char* zookeeper_host, char* addr) {
    return server_create(n_lists, zookeeper_host, addr);    
}

int table_skel_destroy(struct server_t *server) {
    return server_destroy(server);
}

int invoke(MessageT *msg, struct server_t *server) {

    struct timeval start, end;
    long time;

    if (msg == NULL || server == NULL) //check for Null values
        return -1;

    if (msg->opcode == MESSAGE_T__OPCODE__OP_PUT) {

        gettimeofday(&start, NULL);
        server_increase_ops(server);
        
        char* key = strdup(msg->entry->key);

        void* content = malloc(msg->entry->value.len);

        if (content == NULL) {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            gettimeofday(&end, NULL);
            time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            server_increase_time(server, time);
            return -1;

        } //error in malloc

        memcpy(content, msg->entry->value.data, msg->entry->value.len);


        struct data_t* data = data_create(msg->entry->value.len, content);

        
        int result = server_put(server,key, data);

        free(key);
        data_destroy(data);
        
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

        if (result == -1) { //error in op
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            gettimeofday(&end, NULL);
            time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            server_increase_time(server, time);
            return 0;
        }

        msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
        gettimeofday(&end, NULL);
        time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        server_increase_time(server, time);
        return 0;

    }

    else if (msg->opcode == MESSAGE_T__OPCODE__OP_GET) {
        gettimeofday(&start, NULL);
        server_increase_ops(server);

        struct data_t *data = server_get(server,msg->key);

        if (data == NULL) { //error in op
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            gettimeofday(&end, NULL);
            time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            server_increase_time(server, time);
            return 0;
        }

        ProtobufCBinaryData value;
        value.len = data->datasize;

        if ((value.data = malloc(value.len)) == NULL) { //error in malloc
            data_destroy(data);
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            gettimeofday(&end, NULL);
            time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            server_increase_time(server, time);
            return -1;
        }

        if (memcpy(value.data, data->data, value.len) == NULL) { //error in memcpy
            data_destroy(data);
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            gettimeofday(&end, NULL);
            time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            server_increase_time(server, time);
            return -1;
        }


        msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
        msg->value = value;
        data_destroy(data);
        gettimeofday(&end, NULL);
        time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        server_increase_time(server, time);
        return 0;
    }

    else if (msg->opcode == MESSAGE_T__OPCODE__OP_DEL) {
        gettimeofday(&start, NULL);
        server_increase_ops(server);

        int result = server_delete(server, msg->key);
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

        if (result == -1 || result == 1) { //error in op
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            gettimeofday(&end, NULL);
            time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            server_increase_time(server, time);
            return 0;
        }

        msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
        gettimeofday(&end, NULL);
        time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        server_increase_time(server, time);
        return 0;
    }

    else if (msg->opcode == MESSAGE_T__OPCODE__OP_SIZE) {
        gettimeofday(&start, NULL);
        server_increase_ops(server);

        int size = server_size(server);

        if (size == -1) { //error in op
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            gettimeofday(&end, NULL);
            time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            server_increase_time(server, time);
            return 0;
        }

        msg->result = size;
        msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        gettimeofday(&end, NULL);
        time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        server_increase_time(server, time);
        return 0;
    }

    else if (msg->opcode == MESSAGE_T__OPCODE__OP_GETKEYS) {
        gettimeofday(&start, NULL);
        server_increase_ops(server);
        
        char**keys = server_keys(server);

        if(keys == NULL) { //error in op
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            gettimeofday(&end, NULL);
            time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            server_increase_time(server, time);
            return 0;
        }

        msg->keys = keys;
        msg->n_keys = server_size(server);
        msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
        gettimeofday(&end, NULL);
        time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        server_increase_time(server, time);
        return 0;
    }
    
    else if (msg->opcode == MESSAGE_T__OPCODE__OP_GETTABLE) {
        gettimeofday(&start,NULL);
        server_increase_ops(server);

        char** keys = server_keys(server);

        if (keys == NULL) { //error in op
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            gettimeofday(&end, NULL);
            time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            server_increase_time(server, time);
            return 0;
        }

        int size = server_size(server);

        if(size == -1) { //error in size        
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            gettimeofday(&end, NULL);
            time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            server_increase_time(server, time);
            return 0;
        }


        msg->entries = malloc(size * sizeof(EntryT*) + sizeof(NULL));

        if(msg->entries == NULL) { //error in malloc
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            gettimeofday(&end, NULL);
            time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            server_increase_time(server, time);
            return 0;
        }
        

        //fill entries 
        for (size_t i = 0; i < size; i++) {

            msg->entries[i] = malloc(sizeof(EntryT));
            entry_t__init(msg->entries[i]);

            struct data_t *value = server_get(server, keys[i]);

            msg->entries[i]->key = strdup(keys[i]);

            ProtobufCBinaryData data;
            data.len = value->datasize;

            data.data = malloc(data.len);

            memcpy(data.data, value->data, data.len);
            msg->entries[i]->value = data;

            data_destroy(value);
        }

        msg->entries[size] = NULL;

        
        server_free_keys(keys);
        msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
        msg->n_entries = size;
        gettimeofday(&end, NULL);
        time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        server_increase_time(server, time);
        return 0;
        
    }

    else if (msg->opcode == MESSAGE_T__OPCODE__OP_STATS) {
        struct statistics_t* stats = server_stats(server);
        
        if (stats == NULL) { //error in op
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return 0;
        }



        msg->opcode = MESSAGE_T__OPCODE__OP_STATS + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_STATS;

        msg->stats = malloc(sizeof(StatisticsT));
        statistics_t__init(msg->stats);
        msg->stats->n_clients = stats->n_clients;
        msg->stats->op_counter = stats->op_counter;
        msg->stats->time = stats->time;

        stats_destroy(stats);
        return 0;
    }

    //unrecognized operation
    return -1;
}