/**
 * Grupo 51
 * Henrique Vale 58168
 * Leonardo Monteiro 58250
 * Sergio Esteves 58245
*/

#include "network_server.h"
#include "inet.h"
#include <stdlib.h>
#include "table_skel.h"
#include "message.h"
#include <errno.h>

/* Função para preparar um socket de receção de pedidos de ligação
 * num determinado porto.
 * Retorna o descritor do socket ou -1 em caso de erro.
 */

int network_server_init(short port){
    int sockfd;
    struct sockaddr_in server;

    // Create TCP Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {

        perror("Error creating socket");
        return -1;
    }
    
    //allow server to reuse ports
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind
    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Error binding");
        close(sockfd);
        return -1;
    };

    // listen
    if (listen(sockfd, 0) < 0){
        perror("Error in listen");
        close(sockfd);
        return -1;
    };

     
    return sockfd;  //socket descriptor
}


int network_main_loop(int listening_socket, struct server_t *server){
    struct sockaddr_in client;
    socklen_t size_client = sizeof(client);
    int connsockfd;
    printf("Server waiting for connections\n");

    while (1) {
        connsockfd = accept(listening_socket, (struct sockaddr *) &client, &size_client);
        printf("Client Socket %d has connected\n", connsockfd);
        server_increase_clients(server);
        pthread_t thread;
        struct thread_args* args = malloc(sizeof(struct thread_args));
        args->connsockfd = connsockfd;
        args->server = server;
        
        pthread_create(&thread,NULL, &handle_client, (void*) args);
        pthread_detach(thread);
    }
}

void* handle_client(void* params) {
    struct thread_args* args = (struct thread_args*) params;
    MessageT* rcv_msg;
    while ((rcv_msg = network_receive(args->connsockfd)) !=NULL) {

        if (invoke(rcv_msg, args->server) == -1) { //do op on table
            message_t__free_unpacked(rcv_msg, NULL);
            perror("Error in invoke");

        }


        else if (network_send(args->connsockfd, rcv_msg) ==-1) { //error in response
            message_t__free_unpacked(rcv_msg, NULL);
            perror("Error in send");
        }
        message_t__free_unpacked(rcv_msg, NULL);
       
        }
        
    close(args->connsockfd);
    printf("Client Socket %d has disconnected\n", args->connsockfd);
    server_decrease_clients(args->server);
    free(args);
    pthread_exit(0);
}

 MessageT *network_receive(int client_socket){

    MessageT *message = NULL;

    short len = 0;

    //get message size from client
    if(read_all(client_socket, &len, 2) != 2) {
        return NULL;
    }

    len = ntohs(len);
    void* buffer = malloc(len);

    if (buffer == NULL) //error in malloc
        return NULL;

    //get message from client
    if (read_all(client_socket, buffer, len) != len) {
        return NULL;
    }


    message = message_t__unpack(NULL, len, buffer);

    if (message == NULL) { //error in unpacking
        return NULL;
    }

    free(buffer);

    return message;
}

int network_send(int client_socket, MessageT *msg) {

    short len = message_t__get_packed_size(msg);
    void* buffer = malloc (len);

    if (buffer == NULL) //error in malloc
        return -1;

    if (buffer == NULL) {
        perror("Error in malloc");
        return -1;
    }

    if (message_t__pack(msg, buffer) != len) {
        return -1;
    }

    short sent_len = htons(len);

    //send message size to client
    if(write_all(client_socket, &sent_len,2) != 2) {
        return -1;
    }

    //send message to client
    if (write_all(client_socket,buffer,len) != len) {
        return -1;
    }

    free(buffer);
    return 0;
}

int network_server_close(int socket){
    if (socket == -1) {
        perror("Error in socket");
        return -1;
    }

    int closed = close(socket);

    if (closed == 0) {
        return 0;
    } 

    else {
        perror("Error while closing socket");
        return -1;
    }
}