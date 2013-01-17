#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 5223
#define LENGTH_OF_LISTEN_QUEUE 20 
#define BUFFER_SIZE 1024 
#define THREAD_MAX 5

char* susbtr(const char* str, const char* token)
{
	const char* pos = strstr(str, token);
	int len = pos - str;
	char* result = (char*)malloc(len + 1);
	strncpy(result, str, len);
	pos = strrchr(result, '/');
	if (pos) {
		strncpy(result, pos + 1, len);
	}
	result[len] = 0;
	return result;
}

void* talk_to_client(void* connect_fd) {
	int connfd = *((int*)connect_fd);
	char buffer[BUFFER_SIZE];
	
	/*
	bzero(buffer, BUFFER_SIZE);
	strcpy(buffer,"Hello,World! Come form server\n");
	//strcat(buffer,"\n"); //C语言字符串连接
	//发送buffer中的字符串到connfd,实际是给客户端
	send(connfd, buffer, strlen(buffer), 0);
	*/
	
	bzero(buffer,BUFFER_SIZE);
	//接收客户端发送来的信息到buffer中
	int length = recv(connfd, buffer, BUFFER_SIZE, 0);
	if (length < 0) {
		printf("NO_DATA!\n");
		exit(1);
	}
	
	//printf("Socket Num: %d \t %s\n", connfd, buffer);
	printf("Socket Num: %d\n", connfd);

	static char* ok_cmd[5] = {"convert", "doc2pdf", "pdf2swf", "htm2img", "report_export"};
	
	char* result = buffer;
	const char* pos = strstr(buffer, " ");
	if (pos) {
		result = substr(buffer, " ");
	}
	int flag = 0, i = 0;
	for (i = 0; ok_cmd[i] != NULL; i++) {
		if (strcmp(ok_cmd[i], result) == 0) {
			flag = 1;
			break;
		}
	}
	if (flag) {
		printf("%s: ", result);
		printf("%s\n", buffer);
		int status = system(buffer);
		bzero(buffer, BUFFER_SIZE);
		if (status == 0) {
			strcpy(buffer, "1");
		} else {
			strcpy(buffer, "0");
		}
		printf("\n\n");
	} else {
		printf("INVALID\n\n");
		bzero(buffer, BUFFER_SIZE);
		strcpy(buffer, "-1");
	}
	send(connfd, buffer, strlen(buffer), 0);
	
	if (pos) {
		free(result);
		result = 0;
	}
	
	
	//关闭与客户端的连接
	close(connfd);
	pthread_exit(NULL);
}

int main(void) {
 	int sockfd, n, connfd;
 	pthread_t tid;
	pthread_attr_t child_thread_attr;
 	struct sockaddr_in servaddr;

	//设置一个socket地址结构server_addr,代表服务器internet地址, 端口
 	bzero(&servaddr, sizeof(servaddr)); //把一段内存区的内容全部设置为0
 	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//servaddr.sin_addr.s_addr = inet_addr(HOST);
 	servaddr.sin_port = htons(PORT);

	//创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket:");
        exit(1);
    }

 	n = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
 	if (n == -1) {
  		perror("bind:");
  		exit(1);
	}

 	n = listen(sockfd, LENGTH_OF_LISTEN_QUEUE);
 	if (n == -1) {
  		perror("listen:");
  		exit(1);
 	}

	//定义客户端的socket地址结构client_addr
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    //接受一个到server_socket代表的socket的一个连接
    //如果没有连接请求,就等待到有连接请求--这是accept函数的特性
    //accept函数返回一个新的socket,这个socket(new_server_socket)用于同连接到的客户的通信
    //new_server_socket代表了服务器和客户端之间的一个通信通道
    //accept函数把连接到的客户端信息填写到客户端的socket地址结构client_addr中

 	while (1) {
  		connfd = accept(sockfd, (struct sockaddr *)&client_addr, &length);
		if (connfd == -1) {
			printf("accept:");
			exit(1);
		}
		pthread_attr_init(&child_thread_attr);
		pthread_attr_setdetachstate(&child_thread_attr,PTHREAD_CREATE_DETACHED);
  		if (pthread_create(&tid, &child_thread_attr, talk_to_client, (void *)&connfd) == -1) {
			printf("pthread_create Failed : ");
			exit(1);
		}
 	}

	//关闭监听用的socket
	close(sockfd);
 	return 0;
}


