/**
 * Grupo 51
 * Henrique Vale 58168
 * Leonardo Monteiro 58250
 * Sergio Esteves 58245
*/

#include "network_server.h"
#include "table_skel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ifaddrs.h>
#include "inet.h"

int sockfd;
struct server_t* server;


void handle_client_close () {
    //do nothing
}

void handle_close() {
    table_skel_destroy(server);
    network_server_close(sockfd);
    printf("Server closed.\n");
    exit(0);
}


char* get_address() {
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    char* address = (char*)malloc(1024);
    address[0] = '\0';

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET || family == AF_INET6) {
            char addr[1024];
            inet_ntop(family, family == AF_INET ?  //check what family the address belongs to
                      (void*)&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr : //IPv4 or
                      (void*)&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr, //IPv6
                      addr, sizeof(addr));

            if (strcmp(addr, "127.0.0.1") != 0 && strcmp(addr, "::1") != 0) { //non-loopback
                strcpy(address, addr);
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    return address;
}

int main(int argc, char const **argv)
{
    if (argc != 4) {
        printf("Invalid arguments. Usage: ./table_server <port> <n_lists> <zookeeper_IP:port>");
        return -1;
    }


    sockfd = network_server_init(atoi(argv[1])); //estabilish connection

    //handle relevant signals
    signal(SIGPIPE, handle_client_close);
    signal(SIGINT, handle_close);
    
    if (sockfd == -1) {
        perror("Error initiating server");
        return -1;
    }

    char* address = get_address();
    strcat(address, ":");
    strcat(address, argv[1]);
    char* zookeeper = strdup(argv[3]);
    server = table_skel_init(atoi(argv[2]), zookeeper, address);
    free(zookeeper);
    free(address);

    while (1) {
    
        if (network_main_loop(sockfd, server) == -1) {
            //When main_loop is left, client has disconnected
            printf("Client connection closed.\n");
        }

    }
}



