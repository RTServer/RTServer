#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>  
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include "../md5/md5.h"
#include "base.h"

char *RTS_current_datetime() {
    char *timestr = (char *)malloc(20 * sizeof(char));
    time_t t;
    struct tm *nowtime;
    time(&t);
    nowtime = localtime(&t);
    strftime(timestr, 20, "%Y-%m-%d %H:%M:%S", nowtime);
    return timestr;
}

char *RTS_unique() {
    char *unique = (char *)malloc(33 * sizeof(char));
    char timestr[20];
    struct timeval tv;
    gettimeofday(&tv, 0);
    sprintf(timestr, "%u.%u", (unsigned int)tv.tv_sec, (unsigned int)tv.tv_usec);

    MD5_CTX md5;
    MD5Init(&md5);         
    int i;
    unsigned char decrypt[16];    
    MD5Update(&md5, (unsigned char *)timestr, strlen((char *)timestr));
    MD5Final(&md5, decrypt);
    for(i = 0; i < 16; i++) {
        char tmp[3] = {0};
        sprintf(tmp, "%02x", decrypt[i]);
        strcpy(unique + 2*i, tmp);
    }
    return unique;
}

char *RTS_md5(char *str) {
    char *md5str = (char *)malloc(33 * sizeof(char));
    MD5_CTX md5;
    MD5Init(&md5);         
    int i;
    unsigned char decrypt[16];    
    MD5Update(&md5, (unsigned char *)str, strlen((char *)str));
    MD5Final(&md5, decrypt);
    for(i = 0; i < 16; i++) {
        char tmp[3] = {0};
        sprintf(tmp, "%02x", decrypt[i]);
        strcpy(md5str + 2*i, tmp);
    }
    return md5str;
}

char *RTS_rand() {
    char *randstr = (char *)malloc(7 * sizeof(char));
    int i;
    struct timeval tv;
    gettimeofday(&tv, 0);
    srand((unsigned int)tv.tv_sec + (unsigned int)tv.tv_usec);
    for (i = 0; i < 6; i++) {//要想大写字母,将97改成65
        char tmp[2] = {0};
        if (rand() % 2) sprintf(tmp, "%c", 97 + rand() % 26);
        else sprintf(tmp, "%d", rand() % 10);
        strcpy(randstr + i, tmp);
    }
    return randstr;
}

char *RTS_hash(char *password, char *salt) {
    char *md5str = RTS_md5(password);
    char saltpwd[39] = {0};
    strcpy(saltpwd, salt);
    strcat(saltpwd, md5str);
    if (md5str != NULL) {
        free(md5str);
        md5str = NULL;
    }
    return RTS_md5(saltpwd);
}

//再实现一个printf(内核的printf就是这么实现的)
int RTS_printf(const char *fmt, ...) {
    if (RTS_DEBUG == 0) {
        return 0;
    }

    va_list args;
    int i;

    char printbuf[256];
    va_start(args, fmt);
    write(1, printbuf, i = vsprintf(printbuf, fmt, args));
    va_end(args);
    return i;
}