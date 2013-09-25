#include <stdio.h>
#include <string.h>
#include <sys/time.h>  
#include <time.h>
#include "base.h"

int gettimeofday(struct timeval *tv, struct timezone *tz);

char *get_current_time() {
    static char timestr[20] = {0};
    time_t t;
    struct tm *nowtime;
    time(&t);
    nowtime = localtime(&t);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", nowtime);
    return timestr;
}

char *RTS_unique() {
    static char unique[32] = {0};
    char timestr[20];
    struct timeval tv;
    gettimeofday(&tv, 0);
    sprintf(timestr, "%u.%u", (unsigned int)tv.tv_sec, (unsigned int)tv.tv_usec);

    MD5_CTX md5;
    MD5Init(&md5);         
    int i;
    unsigned char decrypt[16];    
    MD5Update(&md5, timestr, strlen((char *)timestr));
    MD5Final(&md5, decrypt);
    for(i = 0; i < 16; i++) {
        char tmp[2] = {0};
        sprintf(tmp, "%02x", decrypt[i]);
        strcpy(unique + 2*i, tmp);
    }
    return unique;
}

void RTS_send(int sockfd, const char *content) {
    send(sockfd, content, strlen(content), 0);
}