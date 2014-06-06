/**
 * gcc -o RTTest RTTest.c lib/client/transport.c lib/json/cJSON.c -lpthread
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "../client/transport.h"

#define MAXBUF 10240

#define RTS_DEBUG 1

//用宏重定义printf
#ifndef RTS_DEBUG
void RTS_printf(const char *fmt, ...) {}
#define printf(fmt, args...) RTS_printf(fmt, ##args)
#endif

void signal_callback_handler(int signum) {
    fprintf(stderr, "Caught signal SIGPIPE %d\n", signum);
}

char *_IP = "127.0.0.1";
int _PORT = 5566;

//定义用户数据结构
typedef struct _USER {
    int id;
    char token[MAX_TOKEN_LENGTH + 1];
}_USER;

//for mulThread
//多线程传递参数为结构体类型
typedef struct ARG {
    int n;
    _USER *_user;
}ARG;

int total = 0;

void *talk_to_server(void *arg) {
    int sockfd, len;
    struct sockaddr_in dest;
    char buf[MAXBUF + 1];
    fd_set rfds;
    struct timeval tv;
    int retval, maxfd = -1;
    int port = 5222;
    ARG *info;
    info = (ARG *)arg;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(errno);
    }

    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(_PORT);
    if(inet_aton(_IP, (struct in_addr *)&dest.sin_addr.s_addr) == 0) {
        perror(_IP);
        exit(errno);
    }

    if(connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
        perror("Connect error");
        close(sockfd);
        return NULL;
        //exit(errno);
    }

    printf("等待连接和数据...\n");

    //向服务器发送一次数据，触发下面的接收事件
    bzero(buf, MAXBUF + 1);
    sprintf(buf, "{\"action\":\"login\",\"name\":\"test%d\",\"password\":\"123456\"}", info->n);
    len = send(sockfd, buf, strlen(buf), 0);
    if(len > 0)
        printf("%d-发送成功-name:test%d\n", sockfd, info->n);
    else {
        printf("%d-发送失败\n", sockfd);
        close(sockfd);
        return NULL;
    }
    while (1) {
        sleep(1);
        //printf("okok%d\n", ++i);

        FD_ZERO(&rfds);
        maxfd = 0;

        FD_SET(sockfd, &rfds);
        if(sockfd > maxfd)
            maxfd = sockfd;

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);

        //printf("%d---%d\n", retval, sockfd);
        if (retval == -1) {
            fprintf(stderr, "select error! %s\n", strerror(errno));              
            break;
        } else if (retval == 0) {
            //printf("no msg,no key, and continue to wait……\n");
            continue;
        } else {
            if (FD_ISSET(sockfd, &rfds)) { 
                bzero(buf, MAXBUF + 1);
                len = recv(sockfd, buf, sizeof(buf), 0);
                if(len > 0) {
                    printf("%d-接收数据:%s\n", sockfd, buf);

                    if (info->_user->id == 0) {
                        _RTS_TRANSPORT_DATA _rts_transport_data;
                        _rts_transport_data = RTS_transport_data_init();
                        if (RTS_transport_data_parse(buf, &_rts_transport_data) == 0) {
                            const char* data = "{\"code\":\"0001\",\"message\":\"数据格式非法\"}";
                            send(sockfd, data, strlen(data), 0);
                        }
                        info->_user->id = _rts_transport_data.id;
                        strncpy(info->_user->token, _rts_transport_data.token, MAX_TOKEN_LENGTH);
                    }

                    //接收到数据就再发，sleep 1秒
                    bzero(buf, MAXBUF + 1);
                    int toid = rand() / (RAND_MAX / total + 1) + 1; //随机发送
                    sprintf(buf, "{\"action\":\"message\",\"token\":\"%s\",\"toid\":%d,\"id\":%d,\"content\":\"send to %d\"}", info->_user->token, toid, info->_user->id, toid);
                    //sprintf(buf, "{\"action\":\"message\",\"token\":\"%s\",\"toid\":11,\"id\":23,\"content\":\"send to 11\"}", "bsh_test_$%1KP@'");
                    len = send(sockfd, buf, strlen(buf), 0);
                    if(len > 0)
                        printf("%d-发送成功-data:%s\n", sockfd, buf);
                    else {
                        printf("%d-发送失败\n", sockfd);
                        break;
                    }
                }else {
                    if(len < 0) 
                        printf("%d-接收失败 错误号:%d，错误信息: '%s'， recv返回值: %d\n", sockfd, errno, strerror(errno), len);
                    else
                        printf("%d-退出连接\n", sockfd);
                    break;
                }
            }                
        }
    }

    printf("%d-关闭连接\n", sockfd);
    close(sockfd);

    return NULL;
}

//批量添加用户
int create_num = 2;
int create_total_num = 60000;
void *create_test_user(void *arg) {
    int sockfd, len;
    struct sockaddr_in dest;
    char buf[MAXBUF + 1];
    fd_set rfds;
    struct timeval tv;
    int retval, maxfd = -1;
    int port = 5222;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(errno);
    }

    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(_PORT);
    if(inet_aton(_IP, (struct in_addr *)&dest.sin_addr.s_addr) == 0) {
        perror(_IP);
        exit(errno);
    }

    if(connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
        perror("Connect error");
        close(sockfd);
        return NULL;
        //exit(errno);
    }

    printf("等待连接和数据...\n");

    //向服务器发送一次数据，触发下面的接收事件
    bzero(buf, MAXBUF + 1);
    sprintf(buf, "{\"action\":\"register\",\"name\":\"test1\",\"password\":\"123456\"}");
    len = send(sockfd, buf, strlen(buf), 0);
    if(len > 0)
        printf("%d-发送成功\n", sockfd);
    else {
        printf("%d-发送失败\n", sockfd);
        close(sockfd);
        return NULL;
    }
    while (1) {
        //printf("okok%d\n", ++i);

        FD_ZERO(&rfds);
        maxfd = 0;

        FD_SET(sockfd, &rfds);
        if(sockfd > maxfd)
            maxfd = sockfd;

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);

        //printf("%d---%d\n", retval, sockfd);
        if (retval == -1) {
            fprintf(stderr, "select error! %s\n", strerror(errno));              
            break;
        } else if (retval == 0) {
            //printf("no msg,no key, and continue to wait……\n");
            continue;
        } else {
            if (FD_ISSET(sockfd, &rfds)) { 
                bzero(buf, MAXBUF + 1);
                len = recv(sockfd, buf, sizeof(buf), 0);
                if(len > 0) {
                    printf("%d-接收数据:%s\n", sockfd, buf);
                    if (create_num > create_total_num) {
                        close(sockfd);
                        return NULL;
                    }

                    bzero(buf, MAXBUF + 1);
                    sprintf(buf, "{\"action\":\"register\",\"name\":\"test%d\",\"password\":\"123456\"}", create_num);
                    len = send(sockfd, buf, strlen(buf), 0);
                    printf("发送成功-data:%s\n", buf);
                    create_num++;
                    if(len > 0)
                        printf("%d-发送成功-data:%s\n", sockfd, buf);
                    else {
                        printf("%d-发送失败\n", sockfd);
                        break;
                    }
                }else {
                    if(len < 0) 
                        printf("%d-接收失败 错误号:%d，错误信息: '%s'\n", sockfd, errno, strerror(errno));
                    else
                        printf("%d-退出连接\n", sockfd);
                    break;
                }
            }                
        }
    }

    close(sockfd);

    return NULL;
}

int main(int argc, char **argv) {
    if(argc != 4) {
        printf("Usage: %s IP Port ThreadNum\n", argv[0]);
        exit(0);
    }
    char *ip = NULL;
    if(argv[1]) {
        _IP = argv[1];
    }
    if(argv[2]) {
        _PORT = atoi(argv[2]);
    }

    signal(SIGPIPE, signal_callback_handler);

    pthread_t child_thread;
    pthread_attr_t child_thread_attr;
    pthread_attr_init(&child_thread_attr);
    pthread_attr_setdetachstate(&child_thread_attr, PTHREAD_CREATE_DETACHED);

    int i, n = atoi(argv[3]), ret;
    total = n;
    for(i = 1; i <= n; i++) {
        _USER *_user = (_USER *)malloc(sizeof(_USER));
        _user->id = 0;
        memset(_user->token, 0, MAX_TOKEN_LENGTH + 1);

        //采用多线程并发3，新线程传递参数变量用指针
        ARG *arg;
        arg = (ARG *)malloc(sizeof(ARG));
        arg->n = i;
        arg->_user = _user;
        ret = pthread_create(&child_thread, &child_thread_attr, talk_to_server, (void *)arg);
        //当创建线程成功时，函数返回0，若不为0则说明创建线程失败，常见的错误返回代码为EAGAIN和EINVAL。
        //前者表示系统限制创建新的线程，例如线程数目过多了；后者表示第二个参数代表的线程属性值非法。
        if(ret != 0)
            fprintf(stderr, "pthread_create Failed : %s\n", strerror(errno));
    }

    //批量添加用户
    //pthread_create(&child_thread, &child_thread_attr, create_test_user, NULL);

    //pthread_join(child_thread, NULL);
    getchar();
    return 0;
}