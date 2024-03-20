/**
 * Grupo 51
 * Henrique Vale 58168
 * Leonardo Monteiro 58250
 * Sergio Esteves 58245
*/


#include "network_client.h"
#include "client_stub-private.h"
#include "message.h"


int network_connect(struct rtable_t *rtable) {
    int sockfd; //descriptor
    struct sockaddr_in server;

    // Create TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    
    rtable->sockfd = sockfd;


    server.sin_family = AF_INET; 
    server.sin_port = htons((rtable->server_port)); 
    if (inet_pton(AF_INET, rtable->server_address, &server.sin_addr) < 1) { 
        close(sockfd);
        return -1;
    }

    if (connect(sockfd,(struct sockaddr *)&server, sizeof(server)) < 0) {
        close(sockfd);
        return -1;
    }

    return 0;

}

MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg) {
    if(rtable == NULL || msg == NULL) //check for Null values
        return NULL;

    short len = message_t__get_packed_size(msg);

    if (len < 0) //error getting size
        return NULL;

    void* buf = malloc(len);

    if (buf == NULL) //error in malloc
        return NULL;

    message_t__pack(msg, buf);

    short sent_len = htons(len);

    if(write_all(rtable->sockfd, &sent_len, 2) != 2) { //send message size
        free(buf);
        return NULL;
    }

    if(write_all(rtable->sockfd, buf, len) != len) { //send message
        free(buf);
        return NULL;
    }

    short rec_len = 0;
    
    if((read_all(rtable->sockfd,&rec_len,2)) != 2){ //read message size
        free(buf);
        return NULL;
    };

    rec_len = ntohs(rec_len);

    buf = realloc (buf,rec_len);

    if (buf == NULL) //error in realloc
        return NULL;

    if((read_all(rtable->sockfd,buf, rec_len)) != rec_len){ //read message
        free(buf);
        return NULL;
    };


    MessageT *received = message_t__unpack(NULL,rec_len, buf);

    free(buf);

    if(received == NULL) { //error in unpacking
        return NULL;
    }
 

    return received;
}


int network_close(struct rtable_t *rtable) {
    if (rtable == NULL) 
        return -1;

    if (close(rtable->sockfd) == -1) 
        return -1;

    return 0;
}