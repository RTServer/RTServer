/**
gcc -o RTServer RTServer.c clientctrl.h clientctrl.c parsexml.h parsexml.c -lpthread `xml2-config --cflags --libs`

服务器的TCP状态(连接状态数量统计)
netstat -n | awk '/^tcp/ {++S[$NF]} END {for(a in S) print a, S[a]}'

Linux中查看socket状态：
cat /proc/net/sockstat #（这个是ipv4的）
sockets: used 137 TCP: inuse 49 orphan 0 tw 3272 alloc 52 mem 46UDP: inuse 1 mem 0RAW: inuse 0 FRAG: inuse 0 memory 0
说明：
sockets: used：已使用的所有协议套接字总量
TCP: inuse：正在使用（正在侦听）的TCP套接字数量。其值≤ netstat –lnt | grep ^tcp | wc –l
TCP: orphan：无主（不属于任何进程）的TCP连接数（无用、待销毁的TCP socket数）
TCP: tw：等待关闭的TCP连接数。其值等于netstat –ant | grep TIME_WAIT | wc –l
TCP：alloc(allocated)：已分配（已建立、已申请到sk_buff）的TCP套接字数量。其值等于netstat –ant | grep ^tcp | wc –l
TCP：mem：套接字缓冲区使用量（单位不详。用scp实测，速度在4803.9kB/s时：其值=11，netstat –ant 中相应的22端口的Recv-Q＝0，Send-Q≈400）
UDP：inuse：正在使用的UDP套接字数量
RAW：
FRAG：使用的IP段数量
IPv6请看：cat /proc/net/sockstat6
TCP6: inuse 3UDP6: inuse 0RAW6: inuse 0 FRAG6: inuse 0 memory 0

查看进程的所有线程
# ps mp 6648 -o THREAD,tid
查看所有子进程：
# pstree -p 6648
查看/proc/pid/status可以看到一些进程的当前状态：
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>

#include "clientctrl.h"

#define MAX_LENGTH 1024

int main(int argc, char** argv) {
    int i, maxi = -1;
    int nready;
    int slisten, sockfd, maxfd = -1, connectfd;
    
    unsigned int myport, lisnum; 

    struct sockaddr_in my_addr, addr;
    struct timeval tv;
    
    socklen_t len;
    fd_set rset, allset;

    //第一个参数：端口号
    if(argv[1]) 
        myport = atoi(argv[1]); //将字符串转换成整形
    else
        myport = 5222;

    //第二个参数：最大监听数
    if(argv[2])
        lisnum = atoi(argv[2]);
    else 
        lisnum = MAX_LENGTH;

    //创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
    if((slisten = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myport);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(slisten, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    //listen(sockfd, LENGTH_OF_LISTEN_QUEUE);
    if(listen(slisten, lisnum) == -1) {
        perror("listen");
        exit(1);
    }

    //初始化客户端对象
    client_init();

    FD_ZERO(&allset);           
    FD_SET(slisten, &allset);  
    maxfd = slisten;
    
    printf("等待连接和数据...\n");
    while(1) {
        rset = allset;            

        tv.tv_sec = 1;
        tv.tv_usec = 0;
        /**
        Select在Socket编程中还是比较重要的，可是对于初学Socket的人来说都不太爱用Select写程序，
        他们只是习惯写诸如connect、accept、recv或recvfrom这样的阻塞程序（所谓阻塞方式block，
        顾名思义，就是进程或是线程执行到这些函数时必须等待某个事件的发生，如果事件没有发生，
        进程或线程就被阻塞，函数不能立即返回）。可是使用Select就可以完成非阻塞（所谓非阻塞方
        式non-block，就是进程或线程执行此函数时不必非要等待事件的发生，一旦执行肯定返回，
        以返回值的不同来反映函数的执行情况，如果事件发生则与阻塞方式相同，若事件没有发生则返回
        一个代码来告知事件未发生，而进程或线程继续执行，所以效率较高）方式工作的程序，它能够监
        视我们需要监视的文件描述符的变化情况——读写或是异常。
        int select(int maxfdp,fd_set *readfds,fd_set *writefds,fd_set *errorfds,struct timeval *timeout);
        */
        nready = select(maxfd + 1, &rset, NULL, NULL, &tv);

        if(nready == 0)
            continue;
        else if(nready < 0) {
            printf("select failed!\n");
            break;
        }else {
            if(FD_ISSET(slisten, &rset)) { // new connection
                len = sizeof(struct sockaddr);
                if((connectfd = accept(slisten, (struct sockaddr*)&addr, &len)) == -1) {
                    perror("accept() error\n");
                    continue;
                }

                if((i = client_add(connectfd, addr)) == -1) { //连接数太多
                    close(connectfd); //退出客户端
                    continue;
                }

                FD_SET(connectfd, &allset);
                if(connectfd > maxfd)
                    maxfd = connectfd;
                if(i > maxi)
                    maxi = i;
            }else {
                for(i = 0; i <= maxi; i++) {   
                    if((sockfd = client_getconfd(i)) < 0)
                        continue;
                    if(FD_ISSET(sockfd, &rset)) { //接收客户端信息
                        if (!client_interface(sockfd, i)) {
                            printf("客户端退出了\n");
                            close(sockfd);
                            FD_CLR(sockfd, &allset);
                            client_clearn(i);
                        }
                    }
                }
            }
        }    
    }
    close(slisten);
}