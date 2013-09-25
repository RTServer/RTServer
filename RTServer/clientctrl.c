#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "lib/json/cJSON.h"
#include "lib/md5/md5.h"
#include <sys/time.h>  
#include <time.h>
int gettimeofday(struct timeval *tv, struct timezone *tz);

#define MAX_CLIENT 1024
#define MAX_BUF 10240
#define MAX_NAME_LENGTH 20
#define MAX_TOKEN_LENGTH 32
#define MAX_CONTENT_LENGTH 1024
#define MAX_ACTION_LENGTH 15
#define MAX_PASSWORD_LENGTH 32

//定义接入客户端数据结构
typedef struct _CLIENT {
    int fd;
    int id;
    char name[MAX_NAME_LENGTH + 1];
    char token[MAX_TOKEN_LENGTH + 1];
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
int client_interface(int sockfd, int i, int maxi);


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
        memset(_client[i].name, 0, MAX_NAME_LENGTH + 1);
        memset(_client[i].token, 0, MAX_TOKEN_LENGTH + 1);
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

void client_print(int maxi) {
    int i;
    for(i = 0; i <= maxi; i++) {
        printf("index:%d--fd:%d--id:%d--name:%s--token:%s\n", i, _client[i].fd, _client[i].id, _client[i].name, _client[i].token);
    }
}

/**
 * 根据id获取客户端索引
 * @param id [description]
 */
int client_getindex(int id, int maxi) {
    int i;
    for(i = 0; i <= maxi; i++) {
        if(_client[i].id == id) {
            return i;
        }
    }
    return -1;
}


char *get_current_time() {
    static char timestr[20] = {0};
    time_t t;
    struct tm *nowtime;
    time(&t);
    nowtime = localtime(&t);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", nowtime);
    return timestr;
}

char *RTS_unique() {
    static char unique[32] = {0};
    char timestr[20];
    struct timeval tv;
    gettimeofday(&tv, 0);
    sprintf(timestr, "%u.%u", tv.tv_sec, tv.tv_usec);

    MD5_CTX md5;
    MD5Init(&md5);         
    int i;
    unsigned char decrypt[16];    
    MD5Update(&md5, timestr, strlen((char *)timestr));
    MD5Final(&md5, decrypt);
    for(i = 0; i < 16; i++) {
        char tmp[2] = {0};
        sprintf(tmp, "%02x", decrypt[i]);
        strcpy(unique + 2*i, tmp);
    }
    return unique;
}

/**
 * 和客户交互
 * @param  sockfd [description]
 * @param  i      [description]
 * @return        [description]
 */
int client_interface(int sockfd, int i, int maxi) {
	bzero(buf, MAX_BUF + 1);
	int n;
    if((n = recv(sockfd, buf, MAX_BUF, 0)) > 0) {
        //printf("第%d个客户端 IP:%s\n", i + 1, inet_ntoa(_client[i].addr.sin_addr));

        //解析客户端信息
        int flag = 0, id = 0, toid = 0;
        char action[MAX_ACTION_LENGTH + 1] = {0};
        char name[MAX_NAME_LENGTH + 1] = {0};
        char password[MAX_PASSWORD_LENGTH + 1] = {0};
        char token[MAX_TOKEN_LENGTH + 1] = {0};
        wchar_t content[MAX_CONTENT_LENGTH + 1] = {0};
        cJSON *json = NULL, *json_tmp = NULL;
        json = cJSON_Parse(buf);
        if (!json) {
            //printf("Error before: [%s]\n", cJSON_GetErrorPtr());
            bzero(buf, MAX_BUF + 1);
            sprintf(buf, "{\"code\":\"0001\",\"message\":\"数据格式非法\"}");
            send(sockfd, buf, strlen(buf), 0);
            flag = 0;
        } else {
            (json_tmp = cJSON_GetObjectItem(json, "action")) && strncpy(action, json_tmp->valuestring, MAX_ACTION_LENGTH);
            if (!action) {
                bzero(buf, MAX_BUF + 1);
                sprintf(buf, "{\"code\":\"0005\",\"message\":\"参数非法\"}");
                send(sockfd, buf, strlen(buf), 0);
                flag = 0;
            } else {
                if (strcmp(action, "login") == 0) { //登录
                    (json_tmp = cJSON_GetObjectItem(json, "name")) && strncpy(name, json_tmp->valuestring, MAX_NAME_LENGTH);
                    (json_tmp = cJSON_GetObjectItem(json, "password")) && strncpy(password, json_tmp->valuestring, MAX_PASSWORD_LENGTH);
                    (json_tmp = cJSON_GetObjectItem(json, "id")) && (id = json_tmp->valueint);
                    if (!name || !password || !id) {
                        bzero(buf, MAX_BUF + 1);
                        sprintf(buf, "{\"code\":\"0005\",\"message\":\"参数非法\"}");
                        send(sockfd, buf, strlen(buf), 0);
                        flag = 0;
                    } else {
                        if ((strcmp(name, "test1") == 0 && strcmp(password, "123456") == 0) || (strcmp(name, "test2") == 0 && strcmp(password, "123456") == 0)) {
                            //登录成功，返回认证标示
                            strncpy(token, RTS_unique(), MAX_TOKEN_LENGTH);
                            _client[i].id = id;
                            strncpy(_client[i].name, name, MAX_NAME_LENGTH);
                            strncpy(_client[i].token, token, MAX_TOKEN_LENGTH);
                            printf("%s登录成功\n", name, inet_ntoa(_client[i].addr.sin_addr));
                            bzero(buf, MAX_BUF + 1);
                            sprintf(buf, "{\"code\":\"0000\",\"message\":\"登录成功\",\"token\":\"%s\"}", token);
                            send(sockfd, buf, strlen(buf), 0);
                            flag = 1;
                        } else {
                            bzero(buf, MAX_BUF + 1);
                            sprintf(buf, "{\"code\":\"0003\",\"message\":\"用户名或密码错误\"}");
                            send(sockfd, buf, strlen(buf), 0);
                            flag = 0;
                        }
                    }
                } else if (strcmp(action, "message") == 0) { //聊天
                    (json_tmp = cJSON_GetObjectItem(json, "token")) && strncpy(token, json_tmp->valuestring, MAX_TOKEN_LENGTH);
                    (json_tmp = cJSON_GetObjectItem(json, "toid")) && (toid = json_tmp->valueint);
                    (json_tmp = cJSON_GetObjectItem(json, "id")) && (id = json_tmp->valueint);
                    (json_tmp = cJSON_GetObjectItem(json, "content")) && mbstowcs(content, json_tmp->valuestring, MAX_CONTENT_LENGTH);
                    if (!token || !toid || !id || !content) {
                        bzero(buf, MAX_BUF + 1);
                        sprintf(buf, "{\"code\":\"0005\",\"message\":\"参数非法\"}");
                        send(sockfd, buf, strlen(buf), 0);
                        flag = 0;
                    } else {
                        //认证信息失败，退出客户端
                        if (strcmp(token, _client[i].token) != 0) {
                            bzero(buf, MAX_BUF + 1);
                            sprintf(buf, "{\"code\":\"0004\",\"message\":\"token非法\"}");
                            send(sockfd, buf, strlen(buf), 0);
                            flag = 0;
                        } else {
                            flag = 1;
                            int index;
                            index = client_getindex(toid, maxi); //获取接受者索引
                            if (index == -1 || _client[index].fd == -1) {
                                bzero(buf, MAX_BUF + 1);
                                sprintf(buf, "{\"code\":\"1002\",\"message\":\"对方不在线\"}");
                                send(sockfd, buf, strlen(buf), 0);
                            } else {
                                if (_client[index].id == id) { //如果接受者是自己，则发出警告
                                    bzero(buf, MAX_BUF + 1);
                                    sprintf(buf, "{\"code\":\"1001\",\"message\":\"不能给自己发消息\"}");
                                    send(sockfd, buf, strlen(buf), 0);
                                } else {
                                    bzero(buf, MAX_BUF + 1);
                                    sprintf(buf, "{\"code\":\"0000\",\"message\":\"发送成功\",\"content\":\"%s\"}", content);
                                    send(_client[index].fd, buf, strlen(buf), 0);
                                }
                            }
                        }
                    }
                } else {
                    bzero(buf, MAX_BUF + 1);
                    sprintf(buf, "{\"code\":\"0002\",\"message\":\"动作非法\"}");
                    send(sockfd, buf, strlen(buf), 0);
                    flag = 0;
                }
            }
        }

        client_print(maxi);
        printf("maxi:%d--i:%d\n", maxi, i);

        //释放内存
        cJSON_Delete(json);
        return flag;
    }

    return 0; //客户端退出时
}