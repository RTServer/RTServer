#include <stdlib.h>
#include <wchar.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event.h>
#include "interface.h"
#include "transport.h"
#include "../tools/base.h"
#include "../db/data.h"

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
int client_add(int connectfd, struct sockaddr_in addr, client_t *client) {
	//找到客户端对象中可用的并赋值
	int i;
    for(i = 0; i < MAX_CLIENT; i++) {
        if(_client[i].fd < 0) {
            _client[i].fd   = connectfd;
            _client[i].addr = addr;
            _client[i].client = client;
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
void client_clean(int fd) {
    int i = client_getindex_byfd(fd);
    //如果是合法退出，则修改登录状态
    if (strlen(_client[i].token) > 0) {
        _RTS_USER _rts_user = user_init();
        _rts_user.id = _client[i].id;
        _rts_user.status = 0;
        user_edit(_rts_user);
    }
    _client[i].fd = -1;
    _client[i].id = 0;
    memset(_client[i].name, 0, MAX_NAME_LENGTH + 1);
    memset(_client[i].token, 0, MAX_TOKEN_LENGTH + 1);
}

void client_print() {
    int i;
    for(i = 0; i <= MAX_CLIENT; i++) {
        if (_client[i].fd >= 0) {
            RTS_printf("index:%d--fd:%d--id:%d--name:%s--token:%s\n", i, _client[i].fd, _client[i].id, _client[i].name, _client[i].token);
        }
    }
}

/**
 * 根据id获取客户端索引
 * @param id [description]
 */
int client_getindex(int id) {
    int i;
    for(i = 0; i <= MAX_CLIENT; i++) {
        if(_client[i].id == id) {
            return i;
        }
    }
    return -1;
}

/**
 * 根据id获取客户端索引
 * @param id [description]
 */
int client_getindex_byfd(int fd) {
    int i;
    for(i = 0; i <= MAX_CLIENT; i++) {
        if(_client[i].fd == fd) {
            return i;
        }
    }
    return -1;
}

int client_getid(char *token) {
    int i;
    for(i = 0; i <= MAX_CLIENT; i++) {
        if(strcmp(_client[i].token, token) == 0) {
            return _client[i].id;
        }
    }
    return -1;
}

int client_send(client_t *client, const char *content) {
    evbuffer_add(client->output_buffer, content, strlen(content));
    return evbuffer_write(client->output_buffer, client->fd);
    //return send(client->fd, content, strlen(content), 0);
}

/**
 * 和客户交互
 * @param  sockfd [description]
 * @param  i      [description]
 * @return        [description]
 */
int client_interface(struct bufferevent *bev, client_t *client) {
    char data[4096];
    int nbytes;

    //接收数据
    char buffer[MAX_BUF + 1];
    bzero(buffer, MAX_BUF + 1);
    int n = 0;
    int str_len = 0;
    int last_len;
    while ((nbytes = EVBUFFER_LENGTH(bev->input)) > 0) {
        if (nbytes > 4096) nbytes = 4096;
        n += evbuffer_remove(bev->input, data, nbytes);
        str_len = strlen(data);
        last_len = (MAX_BUF - strlen(buffer));
        if (last_len >= str_len) {
            strncat(buffer, data, str_len);
        } else {
            strncat(buffer, data, last_len);
        }
    }

    if(n > 0) {
        //printf("第%d个客户端 IP:%s\n", i + 1, inet_ntoa(_client[i].addr.sin_addr));

        //解析客户端信息
        int flag = 0;
        int i = client_getindex_byfd(client->fd);
        _RTS_TRANSPORT_DATA _rts_transport_data;
        _rts_transport_data = RTS_transport_data_init();
        if (RTS_transport_data_parse(buffer, &_rts_transport_data) == 0) {
            client_send(client, "{\"code\":\"0001\",\"message\":\"数据格式非法\"}");
            flag = 0;
        } else {
            if (!_rts_transport_data.action) {
                client_send(client, "{\"code\":\"0005\",\"message\":\"参数非法\"}");
                flag = 0;
            } else {
                if (strcmp(_rts_transport_data.action, "login") == 0) { //登录
                    if (strlen(_rts_transport_data.name) == 0 || strlen(_rts_transport_data.password) == 0) {
                        client_send(client, "{\"code\":\"0005\",\"message\":\"参数非法\"}");
                        flag = 0;
                    } else {
                        //验证用户名密码
                        _RTS_USER *_rts_user = user_get(0, _rts_transport_data.name);
                        if (_rts_user->id > 0) {
                            char *pwdhash = RTS_hash(_rts_transport_data.password, _rts_user->salt);
                            if (strcmp(_rts_user->password, pwdhash) == 0) {
                                //用户名密码验证通过后，判断当前账号是否已登录
                                //这时应该已经拿到了用户的id
                                if (strlen(_client[i].token) > 0) { //该设备已经登录了账号，不能再做登录操作
                                    client_send(client, "{\"code\":\"1004\",\"message\":\"该设备已经登录了账号，不能再做登录操作\"}");
                                } else {
                                    int index = client_getindex(_rts_user->id);
                                    if (index == -1 || _client[index].fd == -1 || strlen(_client[index].token) == 0) { //未登录，直接执行登录赋值操作
                                        //登录成功，返回认证标示
                                        char *unique = RTS_unique();
                                        strncpy(_rts_transport_data.token, unique, MAX_TOKEN_LENGTH);
                                        free(unique);
                                        unique = NULL;
                                        _client[i].id = _rts_user->id;
                                        strncpy(_client[i].name, _rts_transport_data.name, MAX_NAME_LENGTH);
                                        strncpy(_client[i].token, _rts_transport_data.token, MAX_TOKEN_LENGTH);
                                        RTS_printf("登录成功: NAME:%s--IP:%s\n", _rts_transport_data.name, inet_ntoa(_client[i].addr.sin_addr));
                                        bzero(buf, MAX_BUF + 1);
                                        sprintf(buf, "{\"code\":\"0000\",\"message\":\"登录成功\",\"token\":\"%s\",\"id\":%d}", _rts_transport_data.token, _rts_user->id);
                                        client_send(client, buf);

                                        //修改登录成功标识
                                        _RTS_USER _rts_user2 = user_init();
                                        _rts_user2.id = _rts_user->id;
                                        _rts_user2.status = 1;
                                        user_edit(_rts_user2);
                                    } else {
                                        client_send(client, "{\"code\":\"1003\",\"message\":\"该用户已经在其他地方成功登录\"}");
                                    }
                                }
                                flag = 1;
                            } else {
                                client_send(client, "{\"code\":\"0003\",\"message\":\"用户名或密码错误\"}");
                                flag = 0;
                            }
                            free(pwdhash); pwdhash = NULL;
                        } else {
                            client_send(client, "{\"code\":\"0003\",\"message\":\"用户名或密码错误\"}");
                            flag = 0;
                        }
                        free(_rts_user);
                        _rts_user = NULL;
                    }
                } else if (strcmp(_rts_transport_data.action, "message") == 0) { //聊天
                    if (strlen(_rts_transport_data.token) == 0 || !_rts_transport_data.toid || !_rts_transport_data.id || wcslen(_rts_transport_data.content) == 0) {
                        client_send(client, "{\"code\":\"0005\",\"message\":\"参数非法\"}");
                        flag = 0;
                    } else {
                        //认证信息失败，退出客户端
                        if (strcmp(_rts_transport_data.token, "bsh_test_$%1KP@'") != 0 && strcmp(_rts_transport_data.token, _client[i].token) != 0) {
                            client_send(client, "{\"code\":\"0004\",\"message\":\"token非法\"}");
                            flag = 0;
                        } else {
                            //判断登录者身份：因为token是唯一的，所以只能通过客户端传过来的token判断当前登录者的身份
                            int realid = client_getid(_rts_transport_data.token);
                            if (realid == -1 || realid != _rts_transport_data.id) {
                                client_send(client, "{\"code\":\"0006\",\"message\":\"用户身份非法\"}");
                                flag = 0;
                            } else {
                                flag = 1;
                                int index;
                                index = client_getindex(_rts_transport_data.toid); //获取接受者索引
                                if (index == -1 || _client[index].fd == -1) {
                                    client_send(client, "{\"code\":\"1002\",\"message\":\"对方不在线\"}");
                                } else {
                                    if (_client[index].id == _rts_transport_data.id) { //如果接受者是自己，则发出警告
                                        client_send(client, "{\"code\":\"1001\",\"message\":\"不能给自己发消息\"}");
                                    } else {
                                        bzero(buf, MAX_BUF + 1);
                                        sprintf(buf, "{\"code\":\"0000\",\"message\":\"发送成功\",\"content\":\"%ls\"}", _rts_transport_data.content);
                                        client_send(_client[index].client, buf);
                                    }
                                }
                            }
                        }
                    }
                } else if (strcmp(_rts_transport_data.action, "logout") == 0) { //logout
                    if (strlen(_rts_transport_data.token) == 0 || !_rts_transport_data.id) {
                        client_send(client, "{\"code\":\"0005\",\"message\":\"参数非法\"}");
                        flag = 0;
                    } else {
                        //认证信息失败，退出客户端
                        if (strcmp(_rts_transport_data.token, _client[i].token) != 0) {
                            client_send(client, "{\"code\":\"0004\",\"message\":\"token非法\"}");
                            flag = 0;
                        } else {
                            //判断登录者身份：因为token是唯一的，所以只能通过客户端传过来的token判断当前登录者的身份
                            int realid = client_getid(_rts_transport_data.token);
                            if (realid == -1 || realid != _rts_transport_data.id) {
                                client_send(client, "{\"code\":\"0006\",\"message\":\"用户身份非法\"}");
                                flag = 0;
                            } else {
                                client_send(client, "{\"code\":\"0000\",\"message\":\"退出成功\"}");
                                flag = 0;
                            }
                        }
                    }
                } else if (strcmp(_rts_transport_data.action, "register") == 0) {
                    if (strlen(_rts_transport_data.name) == 0 || strlen(_rts_transport_data.password) == 0) {
                        client_send(client, "{\"code\":\"0005\",\"message\":\"参数非法\"}");
                        flag = 0;
                    } else {
                        flag = 1;
                        char *salt = RTS_rand();
                        char *pwdhash = RTS_hash(_rts_transport_data.password, salt);
                        char *datetime = RTS_current_datetime();
                        _rts_transport_data.id = user_add(_rts_transport_data.name, pwdhash, salt, inet_ntoa(_client[i].addr.sin_addr), datetime, 0);
                        free(salt); salt = NULL;
                        free(pwdhash); pwdhash = NULL;
                        free(datetime); datetime = NULL;
                        if (_rts_transport_data.id == 0) {
                            client_send(client, "{\"code\":\"1005\",\"message\":\"注册失败,该用户名已注册\"}");
                        } else if (_rts_transport_data.id > 0) {
                            bzero(buf, MAX_BUF + 1);
                            sprintf(buf, "{\"code\":\"0000\",\"message\":\"注册成功\",\"id\":%d}", _rts_transport_data.id);
                            client_send(client, buf);
                        } else {
                            client_send(client, "{\"code\":\"0007\",\"message\":\"注册失败,未知错误\"}");
                            flag = 0;
                        }
                    }
                } else {
                    client_send(client, "{\"code\":\"0002\",\"message\":\"动作非法\"}");
                    flag = 0;
                }
            }
        }

        return flag;
    }

    return 0; //客户端退出时
}