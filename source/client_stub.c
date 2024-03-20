/**
 * Grupo 51
 * Henrique Vale 58168
 * Leonardo Monteiro 58250
 * Sergio Esteves 58245
*/

#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include <stdlib.h>
#include <string.h>


struct rtable_t *rtable_connect(char *address_port) {


    if (address_port == NULL) //check for null value
        return NULL;
    struct rtable_t *table = malloc(sizeof(struct rtable_t));
    if (table == NULL) //error in malloc
        return NULL;


    char* s = strtok(address_port, ":"); //address

    if (s == NULL)
        return NULL;

    table->server_address = s;

    char* s2 = strtok(NULL, ":"); //port

    if (s2 == NULL) 
        return NULL;

    table->server_port = atoi(s2);

    if(network_connect(table) == -1) { //error connecting to server
        free(table);
        return NULL;
    }

    return table;
}


int rtable_disconnect(struct rtable_t *rtable) {
    if (rtable == NULL) //check for NULL value
        return -1;

    if (network_close(rtable) == -1) { //close connection
        free(rtable);
        return -1;
    }

    free(rtable);

    return 0;
}


int rtable_put(struct rtable_t *rtable, struct entry_t *entry){
    if (rtable == NULL || entry == NULL) //check for null value
        return -1;

    //init entry
    EntryT network_entry;
    entry_t__init(&network_entry);

    network_entry.key = malloc (strlen(entry->key) + 1); //size for key + '\0'
    strcpy(network_entry.key, entry->key); //copy key

    ProtobufCBinaryData data;

    data.len = entry->value->datasize;

    data.data = malloc(data.len);

    if (data.data == NULL) //error in malloc
        return -1;

    memcpy(data.data, entry->value->data, data.len);
    network_entry.value = data;

    //init message
    MessageT message;
    message_t__init(&message);

    message.opcode = MESSAGE_T__OPCODE__OP_PUT;
    message.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
    message.entry = &network_entry;


    MessageT* return_message = network_send_receive(rtable, &message); //send message, receive response

    if (return_message == NULL) { //error sending/receiving message
        free(message.entry->key);
        free (data.data);
        free(entry->value);
        free(entry);
        return -1;
    }

    if(return_message->opcode != MESSAGE_T__OPCODE__OP_PUT + 1) { //error in op
        free(entry->value);
        free(entry);
        free(message.entry->key);
        free (data.data);
        message_t__free_unpacked(return_message, NULL);
        return -1;
    }

    free(entry->value);
    free(entry);
    free(message.entry->key);
    free (data.data);

    message_t__free_unpacked(return_message, NULL);

    return 0;
}

struct data_t *rtable_get(struct rtable_t *rtable, char *key) {

    if (rtable == NULL || key == NULL) //check for NULL values
        return NULL;

    MessageT message;
    message_t__init(&message);

    message.opcode = MESSAGE_T__OPCODE__OP_GET;
    message.c_type = MESSAGE_T__C_TYPE__CT_KEY;

    message.key = malloc(strlen(key) + 1); //alloc space for string + '\0'

    if (message.key == NULL) //error in malloc
        return NULL;

    strcpy(message.key, key);

    MessageT *return_message = network_send_receive(rtable, &message); //send and receive message

    free(message.key);

    if (return_message == NULL) //error in send receive
        return NULL;

    if (return_message->opcode != MESSAGE_T__OPCODE__OP_GET + 1) { //error in op
        message_t__free_unpacked(return_message, NULL);
        return NULL;

    }

    void* content = malloc(return_message->value.len);

    if (content == NULL) //error in malloc
        return NULL;

    memcpy(content, return_message->value.data, return_message->value.len);
    struct data_t* data = data_create(return_message->value.len, content);

    message_t__free_unpacked(return_message, NULL);

    return data;

}

int rtable_del(struct rtable_t *rtable, char *key) {
    if (rtable == NULL || key == NULL) //check for Null values
        return -1;
    
    MessageT message;
    message_t__init(&message);

    message.opcode = MESSAGE_T__OPCODE__OP_DEL;
    message.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    message.key = malloc(strlen(key) + 1);

    if(message.key == NULL)
        return -1;

    strcpy(message.key, key);
    
    MessageT *return_message = network_send_receive(rtable, &message); //send and receive message

    free(message.key);

    if (return_message == NULL) //error in send and receive
        return -1;

    if(return_message->opcode != MESSAGE_T__OPCODE__OP_DEL + 1) { //error in op
        message_t__free_unpacked(return_message, NULL);
        return -1;
    }
    
    message_t__free_unpacked(return_message,NULL);
    return 0;
}


int rtable_size(struct rtable_t *rtable) {
    if (rtable == NULL) //check for Null values
        return -1;

    MessageT message;
    message_t__init(&message);

    message.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *return_message = network_send_receive(rtable, &message); //send and receive

    if (return_message == NULL) //error in send and receive
        return -1;

    if (return_message->opcode == MESSAGE_T__OPCODE__OP_ERROR) { //error in op
        message_t__free_unpacked(return_message, NULL);
        return -1;
    }

    int size = return_message->result;
    message_t__free_unpacked(return_message, NULL);
    return size;
    
}

char **rtable_get_keys(struct rtable_t *rtable) {
    if (rtable == NULL) //check for Null value
        return NULL;

    MessageT message;
    message_t__init(&message);

    message.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *return_message = network_send_receive(rtable, &message); //send and receive message
    
    if (return_message == NULL) //error in send and receive
        return NULL;

    if (return_message->opcode != MESSAGE_T__OPCODE__OP_GETKEYS + 1) { //error in op
        message_t__free_unpacked(return_message, NULL);
        return NULL;
    }



    char**keys = malloc(return_message->n_keys*sizeof(char*) + sizeof(NULL));
    
    if (keys == NULL) //error in malloc
        return NULL;

    //fill message keys
    for (size_t i = 0; i < return_message->n_keys; i++) {
        keys[i] = strdup(return_message->keys[i]);
    }

    keys[return_message->n_keys] = NULL; //Null terminator

    message_t__free_unpacked(return_message, NULL);
    return keys;
}

void rtable_free_keys(char **keys) {
    if (keys == NULL) //checks for Null value
        return;

    //freeing each key stored in keys
    int i = 0;
    while (keys[i] != NULL) {
        free(keys[i]);
        i++;
    }

    free(keys);
}

struct entry_t **rtable_get_table(struct rtable_t *rtable) {
    if (rtable == NULL) //check for Null values
        return NULL;

    MessageT message;
    message_t__init(&message);

    message.opcode = MESSAGE_T__OPCODE__OP_GETTABLE;
    message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *return_message = network_send_receive(rtable, &message); //send and receive message
    
    if (return_message == NULL) // error in send and receive
        return NULL;

    if (return_message->opcode != MESSAGE_T__OPCODE__OP_GETTABLE + 1){ //error in op
        message_t__free_unpacked(return_message, NULL);
        return NULL;
    }

    struct entry_t** entries = malloc(sizeof(entries)* return_message->n_entries + sizeof(NULL)); //memory for storing entries from message

    if (entries == NULL) //error in malloc
        return NULL;

    //copy message's entries to created array
    for (size_t i = 0; i < return_message->n_entries; i++) {

        char* key = strdup(return_message->entries[i]->key);
        void* content = malloc(return_message->entries[i]->value.len);
        memcpy(content, return_message->entries[i]->value.data, return_message->entries[i]->value.len);

        struct data_t* data = data_create(return_message->entries[i]->value.len, content);

        entries[i] = entry_create (key, data);
    }
    

    entries[return_message->n_entries] = NULL; //Null terminator

    message_t__free_unpacked(return_message, NULL);
    return entries;
}


void rtable_free_entries(struct entry_t **entries) {
    if (entries == NULL) //checks for Null value
        return;

    //freeing each key stored in keys
    int i = 0;
    while (entries[i] != NULL) {
        entry_destroy(entries[i]);
        i++;
    }

    free(entries);
}

struct statistics_t *rtable_stats(struct rtable_t *rtable) {
    if (rtable == NULL) //check for NULL value
        return NULL;
    MessageT message;
    message_t__init(&message);

    message.opcode = MESSAGE_T__OPCODE__OP_STATS;
    message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *return_message = network_send_receive(rtable, &message);

    if (return_message == NULL) //error in send receive
        return NULL;
    if (return_message->opcode != MESSAGE_T__OPCODE__OP_STATS + 1) { //error in op
        message_t__free_unpacked(return_message, NULL);
        return NULL;
    }

    struct statistics_t *stats = malloc (sizeof(struct statistics_t));
    stats->n_clients = return_message->stats->n_clients;
    stats->op_counter = return_message->stats->op_counter;
    stats->time = return_message->stats->time;

    message_t__free_unpacked(return_message, NULL);
    return stats;   
    
}
