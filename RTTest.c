/**
 * gcc -o RTTest RTTest.c -lpthread
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>

#define MAXBUF 10240

char *_IP = "127.0.0.1";
int _PORT = 5222;

//for mulThread
//多线程传递参数为结构体类型
typedef struct ARG {
    int n;
}ARG;

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
        return;
        //exit(errno);
    }

    printf("等待连接和数据...\n");

    //向服务器发送一次数据，触发下面的接收事件
    bzero(buf, MAXBUF + 1);
    sprintf(buf, "<?xml version='1.0'?><stream:stream xmlns:stream='http://etherx.jabber.org/streams' id='%d' thread='%d' from='jabbercn.org' version='1.0' xml:lang='en'><stream:features><starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/><mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><mechanism>PLAIN</mechanism></mechanisms><c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://www.process-one.net/en/ejabberd/' ver='k87lIPU+P82FgFI2M+F2/LglysI='/><register xmlns='http://jabber.org/features/iq-register'/></stream:features></stream:stream>", sockfd, info->n);
    len = send(sockfd, buf, strlen(buf), 0);
    if(len > 0)
        printf("%d-发送成功\n", sockfd);
    else {
        printf("%d-发送失败\n", sockfd);
        close(sockfd);
        return;
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
            printf("select error! %s", strerror(errno));              
            break;
        } else if (retval == 0) {
            //printf("no msg,no key, and continue to wait……\n");
            continue;
        } else {
            if (FD_ISSET(sockfd, &rfds)) { 
                bzero(buf, MAXBUF + 1);
                len = recv(sockfd, buf, MAXBUF, 0);
                if(len > 0) {
                    printf("%d-接收数据:%s\n", sockfd, buf);

                    //接收到数据就再发，sleep 1秒
                    sleep(1);
                    bzero(buf, MAXBUF + 1);
                    sprintf(buf, "<?xml version='1.0'?><stream:stream xmlns:stream='http://etherx.jabber.org/streams' id='%d' thread='%d' from='jabbercn.org' version='1.0' xml:lang='en'><stream:features><starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/><mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><mechanism>PLAIN</mechanism></mechanisms><c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://www.process-one.net/en/ejabberd/' ver='k87lIPU+P82FgFI2M+F2/LglysI='/><register xmlns='http://jabber.org/features/iq-register'/></stream:features></stream:stream>", sockfd, info->n);
                    len = send(sockfd, buf, strlen(buf), 0);
                    if(len > 0)
                        printf("%d-发送成功\n", sockfd);
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
    pthread_t child_thread;
    pthread_attr_t child_thread_attr;
    pthread_attr_init(&child_thread_attr);
    pthread_attr_setdetachstate(&child_thread_attr, PTHREAD_CREATE_DETACHED);

    int i, n = atoi(argv[3]), ret;
    for(i = 0; i < n; i++) {
        //采用多线程并发3，新线程传递参数变量用指针
        ARG *arg;
        arg = (ARG *)malloc(sizeof(ARG));
        arg->n = i;
        ret = pthread_create(&child_thread, &child_thread_attr, talk_to_server, (void *)arg);
        //当创建线程成功时，函数返回0，若不为0则说明创建线程失败，常见的错误返回代码为EAGAIN和EINVAL。
        //前者表示系统限制创建新的线程，例如线程数目过多了；后者表示第二个参数代表的线程属性值非法。
        if(ret != 0)
            printf("pthread_create Failed : %s\n", strerror(errno));
    }
    //pthread_join(child_thread, NULL);
    getchar();
    return 0;
}