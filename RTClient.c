/**
gcc -o RTClient RTClient.c -lpthread
*/

#include <netinet/in.h>     // for sockaddr_in
#include <sys/types.h>      // for socket
#include <sys/socket.h>     // for socket
#include <stdio.h>          // for printf
#include <stdlib.h>         // for exit
#include <string.h>         // for bzero
#include <pthread.h>        // for pthread
#include <sys/errno.h>      // for errno

#define HELLO_WORLD_SERVER_PORT 5222 
#define BUFFER_SIZE 1024

char * server_IP = NULL;

void * talk_to_server(void * thread_num) {
    //设置一个socket地址结构client_addr,代表客户机internet地址, 端口
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr)); //把一段内存区的内容全部设置为0
    client_addr.sin_family = AF_INET;    //internet协议族
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY表示自动获取本机地址
    client_addr.sin_port = htons(0);    //0表示让系统自动分配一个空闲端口
    //创建用于internet的流协议(TCP)socket,用client_socket代表客户机socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0) {
        printf("Create Socket Failed!\n");
        pthread_exit(NULL);
        exit(1);
    }
    //把客户机的socket和客户机的socket地址结构联系起来
    if(bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr))) {
        printf("Client Bind Port Failed!\n");
        close(client_socket);
        pthread_exit(NULL);
        exit(1);
    }

    //设置一个socket地址结构server_addr,代表服务器的internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton(server_IP, &server_addr.sin_addr) == 0) { //服务器的IP地址来自程序的参数
        printf("Server IP Address Error!\n");
        close(client_socket);
        pthread_exit(NULL);
        exit(1);
    }
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);
    //向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
    if(connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) == -1) {
        printf("Can Not Connect To %s!\n", server_IP);
        close(client_socket);
        pthread_exit(NULL);
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    //从服务器接收数据到buffer中
    int length = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if(length < 0) {
        printf("Recieve Data From Server %s Failed!\n", server_IP);
        close(client_socket);
        pthread_exit(NULL);
        exit(1);
    }
    printf("From Server %s :\t%s", server_IP, buffer);

    bzero(buffer, BUFFER_SIZE);
    sprintf(buffer, "Hello, World! From Client Thread NUM :\t%d\n", (int)thread_num);
    //向服务器发送buffer中的数据
    send(client_socket, buffer, BUFFER_SIZE, 0);

    //关闭socket
    close(client_socket);
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Usage: ./%s ServerIPAddress\n",argv[0]);
        exit(1);
    }
    server_IP = argv[1];
    
    pthread_t child_thread;
    pthread_attr_t child_thread_attr;
    pthread_attr_init(&child_thread_attr);
    pthread_attr_setdetachstate(&child_thread_attr, PTHREAD_CREATE_DETACHED);
    int i=0;
    for(i = 0; i < 10000; i++) {
        if(pthread_create(&child_thread,&child_thread_attr,talk_to_server, (void *)i) == -1)
            printf("pthread_create Failed : %s\n",strerror(errno));
    }
    return 0;
}