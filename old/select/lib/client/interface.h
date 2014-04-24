#include "const.h"

//定义接入客户端数据结构
typedef struct _CLIENT {
    int fd;
    int id;
    char name[MAX_NAME_LENGTH + 1];
    char token[MAX_TOKEN_LENGTH + 1];
    struct sockaddr_in addr;
}_CLIENT;

//定义全局变量
static _CLIENT _client[MAX_CLIENT]; //模块化全局变量_client
char buf[MAX_BUF + 1];

extern void client_init();
struct sockaddr;
extern int client_add(int connectfd, struct sockaddr_in addr);
extern int client_getconfd(int i);
extern int client_interface(int sockfd, int i, int maxi);
extern void client_clean(int i);
extern void client_print(int maxi);