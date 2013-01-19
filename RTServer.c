/**
gcc -o RTServer RTServer.c -lpthread
*/

#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <pthread.h>
#include <sys/errno.h>    // for errno

#define RTSERVER_PORT    5222 
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define THREAD_MAX    5
void * talk_to_client(void *data) {
    int new_server_socket = *((int*)data);
    char buffer[BUFFER_SIZE];

    bzero(buffer, BUFFER_SIZE);
    strcpy(buffer, "Hello,World! 从服务器来！");
    strcat(buffer, "\n"); //C语言字符串连接
    //发送buffer中的字符串到new_server_socket,实际是给客户端
    send(new_server_socket, buffer, BUFFER_SIZE, 0);

    bzero(buffer, BUFFER_SIZE);
    //接收客户端发送来的信息到buffer中
    int length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);
    //int length = rcv(new_server_socket, buffer, BUFFER_SIZE, 0);
    if(length < 0) {
        perror("Server Recieve Data Failed!\n");
        //exit(1);
    }
    printf("\nSocket Num: %d \t %s", new_server_socket, buffer);

    //关闭与客户端的连接
    close(new_server_socket); 
    pthread_exit(NULL);
}

/**
参数解释：
sck - socket
buf - 接收缓冲区
size-缓冲区大小
time_out-等待时间（按秒计）如果超时则返回
返回值：收到字节数，0表示超时等错误
*/
int rcv(int sck, void * buf, int size, int time_out) {
	if(sck < 1 || !buf || size < 1)
		return 0;
	struct timeval tv = {0, 0};
	struct timeval * ptv = 0;
	if(time_out > 0) {
		tv.tv_sec = time_out;
		ptv = &tv;
	}
	memset(buf, 0, size);
	int r = 0;
	char * b = (char*) buf;
	int sz = size; 
	fd_set rd, er;
	int total = 0;
	time_t t0 = time(0);
	time_t t1 = 0;
	do {
		FD_ZERO(&rd);
		FD_SET(sck, &rd);
		FD_ZERO(&er);
		FD_SET(sck, &er);
		r = select(sck + 1, &rd, 0, &er, ptv);
		if(r == -1) {
			perror("select()");
			return -1;
		}
		if(FD_ISSET(sck, &er)) {
			perror("socket(shutdown)");
			return -1;
		}
		if(FD_ISSET(sck, &rd)) {
			r = recv(sck, b, sz, 0);
			if(r == -1) {
				perror("recv()");
				return -1;
			}
			total += r; sz -= r; b+= r;
		}
		if (time_out > 0)
			t1 = time(0) - t0;
		else
			t1 = time_out - 1;
	}while(sz && t1 < time_out);

	return total;
}

int main(int argc, char **argv) {
    //设置一个socket地址结构server_addr,代表服务器internet地址, 端口
    struct sockaddr_in server_addr;

    bzero(&server_addr, sizeof(server_addr)); //把一段内存区的内容全部设置为0
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(RTSERVER_PORT);

    //创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0) {
        printf("Create Socket Failed!");
        exit(1);
    }
    
    //把socket和socket地址结构联系起来
    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
        printf("Server Bind Port : %d Failed!", RTSERVER_PORT); 
        exit(1);
    }
    
    //server_socket用于监听
    if(listen(server_socket, LENGTH_OF_LISTEN_QUEUE)) {
        printf("Server Listen Failed!"); 
        exit(1);
    }
    
    
    while(1) //服务器端要一直运行
    {
        //定义客户端的socket地址结构client_addr
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);

        //接受一个到server_socket代表的socket的一个连接
        //如果没有连接请求,就等待到有连接请求--这是accept函数的特性
        //accept函数返回一个新的socket,这个socket(new_server_socket)用于同连接到的客户的通信
        //new_server_socket代表了服务器和客户端之间的一个通信通道
        //accept函数把连接到的客户端信息填写到客户端的socket地址结构client_addr中
        int new_server_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
        if(new_server_socket < 0) {
            printf("Server Accept Failed!\n");
            break;
        }

        //多线程处理服务器和客户端通信
        pthread_t child_thread;
        pthread_attr_t child_thread_attr;
        pthread_attr_init(&child_thread_attr);
        pthread_attr_setdetachstate(&child_thread_attr, PTHREAD_CREATE_DETACHED);
        if(pthread_create(&child_thread, &child_thread_attr, talk_to_client, (void *)&new_server_socket) == -1)
            printf("pthread_create Failed : %s\n", strerror(errno));
    }

    //关闭监听用的socket
    close(server_socket);
    return 0;
}