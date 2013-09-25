#include <arpa/inet.h>

extern void client_init();
struct sockaddr;
extern int client_add(int connectfd, struct sockaddr_in addr);
extern int client_getconfd(int i);
extern int client_interface(int sockfd, int i, int maxi);
extern void client_clean(int i);
extern void client_print(int maxi);