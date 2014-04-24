#include "const.h"

/**
 * Struct to carry around connection (client)-specific data.
 */
typedef struct client {
    /* The client's socket. */
    int fd;

    /* The event_base for this client. */
    struct event_base *evbase;

    /* The bufferedevent for this client. */
    struct bufferevent *buf_ev;

    /* The output buffer for this client. */
    struct evbuffer *output_buffer;

    /* Here you can add your own application-specific attributes which
     * are connection-specific. */
} client_t;

//定义接入客户端数据结构
typedef struct _CLIENT {
    int fd;
    int id;
    void *user_data;
    char name[MAX_NAME_LENGTH + 1];
    char token[MAX_TOKEN_LENGTH + 1];
    struct sockaddr_in addr;
}_CLIENT;

//定义全局变量
static _CLIENT _client[MAX_CLIENT]; //模块化全局变量_client
char buf[MAX_BUF + 1];

extern void client_init();
struct sockaddr;
extern int client_add(int connectfd, struct sockaddr_in addr, client_t *client);
extern int client_getconfd(int i);
extern int client_interface(int sockfd, int i, int maxi);
extern void client_clean(int i);
extern void client_print(int maxi);