#include <stdio.h>
#include "stdlib.h"
#include <string.h>
#include <sys/time.h>  
#include <time.h>
#include "../md5/md5.h"
#include "base.h"

int gettimeofday(struct timeval *tv, struct timezone *tz);

char *RTS_current_datetime() {
    char *timestr = (char *)malloc(20 * sizeof(char));
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
        char tmp[3] = {0};
        sprintf(tmp, "%02x", decrypt[i]);
        strcpy(unique + 2*i, tmp);
    }
    return unique;
}

char *RTS_md5(char *str) {
    static char md5str[32] = {0};
    MD5_CTX md5;
    MD5Init(&md5);         
    int i;
    unsigned char decrypt[16];    
    MD5Update(&md5, str, strlen((char *)str));
    MD5Final(&md5, decrypt);
    for(i = 0; i < 16; i++) {
        char tmp[3] = {0};
        sprintf(tmp, "%02x", decrypt[i]);
        strcpy(md5str + 2*i, tmp);
    }
    return md5str;
}

char *RTS_rand() {
    static char randstr[6] = {0};
    int i;
    //srand((unsigned)time(NULL)); //这样1s内的随机数就一样了，所以注释掉了
    for (i = 0; i < 6; i++) {//要想大写字母,将97改成65
        char tmp[2] = {0};
        if (rand() % 2) sprintf(tmp, "%c", 97 + rand() % 26);
        else sprintf(tmp, "%d", rand() % 10);
        strcpy(randstr + i, tmp);
    }
    return randstr;
}

char *RTS_hash(char *password, char *salt) {
    return RTS_md5(strcat(salt, RTS_md5(password)));
}

void RTS_send(int sockfd, const char *content) {
    send(sockfd, content, strlen(content), 0);
}