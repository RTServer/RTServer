#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "parsexml.h"

#define MAX_CLIENT 1024
#define MAX_BUF 1024

typedef struct _CLIENT {
    int fd;
    struct sockaddr_in addr;
}_CLIENT;

static _CLIENT client[MAX_CLIENT]; //看看这个值多大 printf("%d\n", FD_SETSIZE);  1024

char buf[MAX_BUF + 1];

/**
 初始化客户端结构
*/
void client_init() {
	int i;
	for(i = 0; i < MAX_CLIENT; i++) {
        client[i].fd = -1;
    }
}

/**
 添加客户端
*/
int client_add(int connectfd, struct sockaddr_in addr) {
	//找到客户端对象中可用的并赋值
	int i;
    for(i = 0; i < MAX_CLIENT; i++) {
        if(client[i].fd < 0) {
            client[i].fd = connectfd;
            client[i].addr = addr;
            //打印客户端ip             
            printf("有客户端接入了 IP:%s\n", inet_ntoa(client[i].addr.sin_addr));
            break;
        }
    }

    if(i == MAX_CLIENT) {
    	bzero(buf, MAX_BUF + 1);
    	sprintf(buf, "连接数太多了");
    	send(connectfd, buf, strlen(buf), 0);
    	return -1; //连接数太多 
    }
    return i;
}

/**
 获取客户端连接标示
*/
int client_getconfd(int i) {
	return client[i].fd;
}

/**
 清理客户端连接标示
*/
void client_clearn(int i) {
	client[i].fd = -1;
}

/**
 和客户交互
*/
int client_interface(int sockfd, int i) {
	bzero(buf, MAX_BUF + 1);
	int n;
    if((n = recv(sockfd, buf, MAX_BUF, 0)) > 0) {
        //printf("第%d个客户端 IP:%s\n", i + 1, inet_ntoa(client[i].addr.sin_addr));
        //解析XML(有问题)
        //printf("CLIENT-%d-接收数据:%s\n", sockfd, buf);
        //xml_parse(buf);

        if(n == 120) {
            //1.接收客户端消息
            printf("CLIENT-%d-接收数据:%s\n", sockfd, buf);

            //2.向客户端发消息
            bzero(buf, MAX_BUF + 1);
            int id = 26323;
            sprintf(buf, "<?xml version='1.0'?><stream:stream xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' id='%d' from='192.168.180.128' version='1.0' xml:lang='en'><stream:features><starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/><mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><mechanism>PLAIN</mechanism></mechanisms><c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://www.process-one.net/en/ejabberd/' ver='k87lIPU+P82FgFI2M+F2/LglysI='/><register xmlns='http://jabber.org/features/iq-register'/></stream:features>", id);
            printf("SERVER-s-发送数据:%s\n", buf);
            send(sockfd, buf, strlen(buf), 0);

            //3.一次交互结束
            printf("\n");
        }else if(n == 51) {
            //1.接收客户端消息
            printf("CLIENT-%d-接收数据:%s\n", sockfd, buf);

            //2.向客户端发消息
            bzero(buf, MAX_BUF + 1);
            sprintf(buf, "<proceed xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
            printf("SERVER-s-发送数据:%s\n", buf);
            send(sockfd, buf, strlen(buf), 0);

            //3.一次交互结束
            printf("\n");
        }else if(n == 16) {
            //同样的方法进行测试
        }else { //实现两个客户端通信
            printf("CLIENT-%d-发送的数据:%s\n", sockfd, buf);
            int to;
            if(i == 0) {
                to = client[1].fd;
            }else if (i == 1) {
                to = client[0].fd;
            }   
            send(to, buf, strlen(buf), 0);                            
        }
        return 1;
    }

    return 0; //客户端退出时
}
