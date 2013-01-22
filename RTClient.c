/**
gcc -o RTClient RTClient.c -lpthread

可以用jabbercn.org(113.105.65.227)这个公共服务测试
./RTClient 113.105.65.227 5222
C:<stream:stream to="jabbercn.org" xmlns="jabber:client" xmlns:stream="http://etherx.jabber.org/streams" version="1.0">
S:<?xml version='1.0'?><stream:stream xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' id='78329114' from='jabbercn.org' version='1.0' xml:lang='en'><stream:features><starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/><mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><mechanism>PLAIN</mechanism></mechanisms><c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://www.process-one.net/en/ejabberd/' ver='k87lIPU+P82FgFI2M+F2/LglysI='/><register xmlns='http://jabber.org/features/iq-register'/></stream:features>

C:<starttls xmlns="urn:ietf:params:xml:ns:xmpp-tls"/>
S:<proceed xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>
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

#define MAXBUF 1024

int main(int argc, char **argv) {
    int sockfd, len;
    struct sockaddr_in dest;
    char buf[MAXBUF + 1];
    fd_set rfds;
    struct timeval tv;
    int retval, maxfd = -1;

    if(argc != 3) {
        printf("Usage: %s IP Port", argv[0]);
        exit(0);
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(errno);
    }

    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(atoi(argv[2]));
    if(inet_aton(argv[1], (struct in_addr *)&dest.sin_addr.s_addr) == 0) {
        perror(argv[1]);
        exit(errno);
    }

    if(connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
        perror("Connect ");
        exit(errno);
    }

    printf("等待连接和数据...\n");        
    while (1) {

        FD_ZERO(&rfds);           
        FD_SET(0, &rfds);
        maxfd = 0;

        FD_SET(sockfd, &rfds);
        if(sockfd > maxfd)
            maxfd = sockfd;

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);

        if (retval == -1) {
            printf("select error! %s", strerror(errno));              
            break;
        } else if (retval == 0) {
             //printf("no msg,no key, and continue to wait……\n"); 
            continue;
        } else {
            if(FD_ISSET(0, &rfds)) {                
                bzero(buf, MAXBUF + 1);
                fgets(buf, MAXBUF, stdin);                
                if(!strncasecmp(buf, "quit", 4)) {
                    printf("退出连接\n");
                    break;
                }
                len = send(sockfd, buf, strlen(buf) - 1, 0);
                if(len > 0)
                    printf("发送成功 长度:%d\n", len);
                else {
                    printf("发送失败\n");
                    break;
                }
            }else if (FD_ISSET(sockfd, &rfds)) { 
                bzero(buf, MAXBUF + 1);
                len = recv(sockfd, buf, MAXBUF, 0);
                if(len > 0)
                    printf("接收数据:%s\n", buf);
                else {
                    if(len < 0) 
                        printf("接收失败 错误号:%d，错误信息: '%s'\n", errno, strerror(errno));
                    else
                        printf("退出连接\n");
                    break;
                }
            }                
        }
    }

    close(sockfd);
    return 0;
}