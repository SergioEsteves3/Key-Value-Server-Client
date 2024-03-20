/**
 * Grupo 51
 * Henrique Vale 58168
 * Leonardo Monteiro 58250
 * Sergio Esteves 58245
*/

#include "message.h"
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>



int write_all(int sock, void *buf, int len) {
    int bufsize = len;
    while(len>0) {
        int res = write(sock, buf, len);
        if(res<0) {
            if(errno==EINTR) continue;
            return res; /* Error != EINTR */
    }
    if(res==0) return res; /* Socket was closed */
        buf += res;
        len -= res;
    }

    return bufsize;
}


int read_all(int sock, void *buf, int len) {
    int bufsize = len;
    while(len>0) {
        int res = read(sock, buf, len);
        if(res<0) {
            if(errno==EINTR) continue;
            return res; /* Error != EINTR */
    }
    if(res==0) return res; /* Socket was closed */
        buf += res;
        len -= res;
    }

    return bufsize;
}