#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "lib/json/cJSON.h"

#define MAX_CLIENT 1024
#define MAX_BUF 10240

//定义接入客户端数据结构
typedef struct _CLIENT {
    int fd;
    int id;
    char *name;
    char *token;
    struct sockaddr_in addr;
}_CLIENT;

//定义全局变量
static _CLIENT _client[MAX_CLIENT]; //模块化全局变量_client
char buf[MAX_BUF + 1];

/**
* 函数声明
*/
void client_init();
int client_add(int connectfd, struct sockaddr_in addr);
int client_getconfd(int i);
void client_clean(int i);
int client_interface(int sockfd, int i);


/**
 * 函数体
 */

/**
 * 初始化客户端结构
 */
void client_init() {
	int i;
	for(i = 0; i < MAX_CLIENT; i++) {
        _client[i].fd = -1;
    }
}

/**
 * 添加客户端
 * @param  connectfd [description]
 * @param  addr      [description]
 * @return           [description]
 */
int client_add(int connectfd, struct sockaddr_in addr) {
	//找到客户端对象中可用的并赋值
	int i;
    for(i = 0; i < MAX_CLIENT; i++) {
        if(_client[i].fd < 0) {
            _client[i].fd   = connectfd;
            _client[i].addr = addr;
            //打印客户端ip             
            //printf("有客户端接入了 IP:%s\n", inet_ntoa(_client[i].addr.sin_addr));
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
 * 获取客户端连接标示
 * @param  i [description]
 * @return   [description]
 */
int client_getconfd(int i) {
	return _client[i].fd;
}

/**
 * 清理客户端连接标示
 * @param i [description]
 */
void client_clean(int i) {
	_client[i].fd = -1;
}

/**
 * 根据id获取客户端索引
 * @param id [description]
 */
int client_getindex(int id) {
    int i;
    for(i = 0; i < MAX_CLIENT; i++) {
        if(_client[i].id == id) {
            return i;
        }
    }
    return -1;
}

/**
 * 和客户交互
 * @param  sockfd [description]
 * @param  i      [description]
 * @return        [description]
 */
int client_interface(int sockfd, int i) {
	bzero(buf, MAX_BUF + 1);
	int n;
    if((n = recv(sockfd, buf, MAX_BUF, 0)) > 0) {
        //printf("第%d个客户端 IP:%s\n", i + 1, inet_ntoa(_client[i].addr.sin_addr));

        //解析客户端信息
        int id, toid;
        char *action, *name, *password, *token, *content;
        cJSON *json;
        json = cJSON_Parse(buf);
        if (!json) {printf("Error before: [%s]\n", cJSON_GetErrorPtr());}
        else {
            action = cJSON_GetObjectItem(json, "action")->valuestring;
            if (strcmp(action, "login") == 0) { //登录
                name = cJSON_GetObjectItem(json, "name")->valuestring;
                password = cJSON_GetObjectItem(json, "password")->valuestring;
                id = cJSON_GetObjectItem(json, "id")->valueint;
                if ((strcmp(name, "test1") == 0 && strcmp(password, "123456") == 0) || (strcmp(name, "test2") == 0 && strcmp(password, "123456") == 0)) {
                    //登录成功，返回认证标示
                    char *token = "abcdefg";
                    _client[i].id = id;
                    _client[i].name = name;
                    _client[i].token = token;
                    printf("%s登录成功\n", name, inet_ntoa(_client[i].addr.sin_addr));
                    bzero(buf, MAX_BUF + 1);
                    sprintf(buf, "{\"code\":\"0000\",\"message\":\"登录成功\",\"token\":\"%s\"}", token);
                    send(sockfd, buf, strlen(buf), 0);
                    return 1;
                } else {
                    bzero(buf, MAX_BUF + 1);
                    sprintf(buf, "{\"code\":\"1001\",\"message\":\"用户名或密码错误\"}");
                    send(sockfd, buf, strlen(buf), 0);
                    return 0;
                }
            } else if (strcmp(action, "message") == 0) { //聊天
                token = cJSON_GetObjectItem(json, "token")->valuestring;
                toid = cJSON_GetObjectItem(json, "toid")->valueint;
                id = cJSON_GetObjectItem(json, "id")->valueint;
                content = cJSON_GetObjectItem(json, "content")->valuestring;
                int index;
                index = client_getindex(toid);
                if (index != -1) {
                    //认证信息失败，退出客户端
                    if (strcmp(token, _client[i].token) != 0) {
                        bzero(buf, MAX_BUF + 1);
                        sprintf(buf, "{\"code\":\"1002\",\"message\":\"token非法\"}");
                        send(sockfd, buf, strlen(buf), 0);
                        return 0;
                    }

                    //判断不是自己
                    if (_client[index].id == id) {
                        return 1;
                    }

                    bzero(buf, MAX_BUF + 1);
                    sprintf(buf, "{\"code\":\"0000\",\"message\":\"发送成功\",\"content\":\"%s\"}", content);
                    send(_client[index].fd, buf, strlen(buf), 0);
                    return 1;
                }
                return 0;
            } else {
                bzero(buf, MAX_BUF + 1);
                sprintf(buf, "{\"code\":\"0002\",\"message\":\"动作非法\"}");
                send(sockfd, buf, strlen(buf), 0);
                return 0;
            }
            

            cJSON_Delete(json);
        }

        bzero(buf, MAX_BUF + 1);
        sprintf(buf, "{\"code\":\"0001\",\"message\":\"未知错误\"}");
        send(sockfd, buf, strlen(buf), 0);
        return 0;
    }

    return 0; //客户端退出时
}