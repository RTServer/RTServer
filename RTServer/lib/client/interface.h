#define MAX_CLIENT 1024
#define MAX_BUF 10240
#define MAX_NAME_LENGTH 20
#define MAX_TOKEN_LENGTH 32
#define MAX_CONTENT_LENGTH 1024
#define MAX_ACTION_LENGTH 15
#define MAX_PASSWORD_LENGTH 32

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