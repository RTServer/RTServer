#include <assert.h>
#include <stdlib.h>
#include <wchar.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event.h>
#include <pthread.h>
#include "interface.h"
#include "transport.h"
#include "../tools/base.h"
#include "../db/data.h"
#include "../hashmap/hashmap.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct client_elem_t {
    char key[20]; //必须的字段
    int id;
    void *client;
    char *name;
    char *token;
    struct sockaddr_in addr;
} client_elem;

typedef struct client_id_elem_t {
    char key[20]; //必须的字段
    int fd;
} client_id_elem;


static int iter_elem(void* elem, void *arg) {
    client_elem *c_el = (client_elem *) elem;
    //printf("fd=%s; addr=%s\n", c_el->key, inet_ntoa(c_el->addr.sin_addr));
    return 0;
}

static int free_client_elem(void* elem, void *arg) {
    client_elem *c_el = (client_elem *) elem;
    //CLIENT_FREE(c_el->client); //这个不能释放
    CLIENT_FREE(c_el->name);
    CLIENT_FREE(c_el->token);
    CLIENT_FREE(c_el);
    return 0;
}

//hashmap存储连接标示对应的客户端信息
hmap_t client_map;

//hashmap存储用户id对应的客户端标示
hmap_t client_id_map;


/**
 * 初始化客户端结构
 */
void client_init() {
    client_map = hashmap_create();
    client_id_map = hashmap_create();
}

/**
 * 添加客户端
 * @param  connectfd [description]
 * @param  addr      [description]
 * @return           [description]
 */
int client_add(int connectfd, struct sockaddr_in addr, client_t *client) {
    /*加锁*/
    if (pthread_mutex_lock(&mutex) != 0) {
        perror("pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }

    if (hashmap_size(client_map) >= MAX_CLIENT) {
        bzero(buf, MAX_BUF + 1);
        sprintf(buf, "连接数太多了");
        send(connectfd, buf, strlen(buf), 0);

        /*解锁锁*/
        if (pthread_mutex_unlock(&mutex) != 0) {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }

        return -1; //连接数太多 
    }

    int ret;
    client_elem *c_el = (client_elem *)malloc(sizeof(client_elem));
    
    sprintf(c_el->key, "%d", connectfd);
    c_el->addr = addr;
    c_el->client = client;
    ret = hashmap_put(client_map, c_el->key, c_el);
    assert(ret == HMAP_S_OK);

    printf("hashmap_size: %d fd:%s\n", hashmap_size(client_map), c_el->key);

    /*解锁锁*/
    if (pthread_mutex_unlock(&mutex) != 0) {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }

    return hashmap_size(client_map);
}

/**
 * 清理客户端连接标示
 * @param i [description]
 */
void client_clean(int fd) {
    /*加锁*/
    if (pthread_mutex_lock(&mutex) != 0) {
        perror("pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }

    //清除client_map
    int ret;
    char key[20];
    sprintf(key, "%d", fd);
    client_elem *c_el;
    ret = hashmap_remove(client_map, key, (void **)&c_el);
    assert(ret == HMAP_S_OK);

    //如果是合法退出，则修改登录状态
    if (c_el->token && strlen(c_el->token) > 0) {
        _RTS_USER _rts_user = user_init();
        _rts_user.id = c_el->id;
        _rts_user.status = 0;
        user_edit(_rts_user);

        //清除client_id_map
        bzero(key, 20);
        sprintf(key, "%d", c_el->id);
        ret = hashmap_remove(client_id_map, key, NULL);
        assert(ret == HMAP_S_OK);

        RTS_printf("[--退出成功--]: NAME:%s--IP:%s\n", c_el->name, inet_ntoa(c_el->addr.sin_addr));
    }
    free_client_elem(c_el, 0);

    /*解锁锁*/
    if (pthread_mutex_unlock(&mutex) != 0) {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }
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
    /*加锁*/
    if (pthread_mutex_lock(&mutex) != 0) {
        perror("pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }

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

    if (n <= 0) {
        /*解锁锁*/
        if (pthread_mutex_unlock(&mutex) != 0) {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }

        return 0; //客户端退出时
    }

    //printf("第%d个客户端 IP:%s\n", i + 1, inet_ntoa(_client[i].addr.sin_addr));

    //解析客户端信息
    int flag = 0;

    int ret;
    char key[20];
    sprintf(key, "%d", client->fd);
    client_elem *c_el;
    ret = hashmap_get(client_map, key, (void **)&c_el);
    assert(ret == HMAP_S_OK);

    _RTS_TRANSPORT_DATA _rts_transport_data;
    _rts_transport_data = RTS_transport_data_init();
    if (RTS_transport_data_parse(buffer, &_rts_transport_data) == 0) {
        client_send(client, "{\"code\":\"0001\",\"message\":\"数据格式非法\"}");

        /*解锁锁*/
        if (pthread_mutex_unlock(&mutex) != 0) {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }

        return 0;
    }

    if (!_rts_transport_data.action) {
        client_send(client, "{\"code\":\"0005\",\"message\":\"参数非法\"}");

        /*解锁锁*/
        if (pthread_mutex_unlock(&mutex) != 0) {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }

        return 0;
    }

    if (strcmp(_rts_transport_data.action, "login") == 0) { //登录
        if (strlen(_rts_transport_data.name) == 0 || strlen(_rts_transport_data.password) == 0) {
            client_send(client, "{\"code\":\"0005\",\"message\":\"参数非法\"}");

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 0;
        }

        //验证用户名密码
        _RTS_USER *_rts_user = user_get(0, _rts_transport_data.name);
        if (_rts_user->id <= 0) {
            client_send(client, "{\"code\":\"0003\",\"message\":\"用户名或密码错误\"}");
            CLIENT_FREE(_rts_user);

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 0;
        }

        char *pwdhash = RTS_hash(_rts_transport_data.password, _rts_user->salt);

        if (strcmp(_rts_user->password, pwdhash) != 0) {
            client_send(client, "{\"code\":\"0003\",\"message\":\"用户名或密码错误\"}");
            CLIENT_FREE(pwdhash);
            CLIENT_FREE(_rts_user);

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 0;
        }

        //用户名密码验证通过后，判断当前账号是否已登录
        //这时应该已经拿到了用户的id
        if (c_el->token && strlen(c_el->token) > 0) { //该设备已经登录了账号，不能再做登录操作
            client_send(client, "{\"code\":\"1004\",\"message\":\"该设备已经登录了账号，不能再做登录操作\"}");
            CLIENT_FREE(pwdhash);
            CLIENT_FREE(_rts_user);

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 1;
        }

        if (c_el->id == 0 || (c_el->token && strlen(c_el->token) == 0)) { //未登录，直接执行登录赋值操作
            //登录成功，返回认证标示
            char *unique = RTS_unique();
            strncpy(_rts_transport_data.token, unique, MAX_TOKEN_LENGTH);
            CLIENT_FREE(unique);

            //重新设置client_map
            ret = hashmap_remove(client_map, c_el->key, (void **)&c_el);
            assert(ret == HMAP_S_OK);
            client_elem *new_c_el = (client_elem *)malloc(sizeof(client_elem));
            sprintf(new_c_el->key, "%d", client->fd);
            new_c_el->addr = c_el->addr;
            new_c_el->client = client;
            new_c_el->id = _rts_user->id;
            new_c_el->name = (char *)malloc(MAX_NAME_LENGTH + 1);
            strncpy(new_c_el->name, _rts_transport_data.name, MAX_NAME_LENGTH);
            new_c_el->token = (char *)malloc(MAX_TOKEN_LENGTH + 1);
            strncpy(new_c_el->token, _rts_transport_data.token, MAX_TOKEN_LENGTH);
            free_client_elem(c_el, 0);
            ret = hashmap_put(client_map, new_c_el->key, new_c_el);
            assert(ret == HMAP_S_OK);

            //设置client_id_map
            client_id_elem *cid_el = (client_id_elem *)malloc(sizeof(client_id_elem));
            sprintf(cid_el->key, "%d", _rts_user->id);
            cid_el->fd = client->fd;
            ret = hashmap_put(client_id_map, cid_el->key, cid_el);
            assert(ret == HMAP_S_OK);

            RTS_printf("[++登录成功++]: NAME:%s--IP:%s\n", _rts_transport_data.name, inet_ntoa(c_el->addr.sin_addr));
            bzero(buf, MAX_BUF + 1);
            sprintf(buf, "{\"code\":\"0000\",\"message\":\"登录成功\",\"token\":\"%s\",\"id\":%d}", _rts_transport_data.token, _rts_user->id);
            client_send(client, buf);

            //修改登录成功标识
            _RTS_USER _rts_user2 = user_init();
            _rts_user2.id = _rts_user->id;
            _rts_user2.status = 1;
            user_edit(_rts_user2);

            CLIENT_FREE(pwdhash);
            CLIENT_FREE(_rts_user);

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 1;
        }

        client_send(client, "{\"code\":\"1003\",\"message\":\"该用户已经在其他地方成功登录\"}");
        CLIENT_FREE(pwdhash);
        CLIENT_FREE(_rts_user);

        /*解锁锁*/
        if (pthread_mutex_unlock(&mutex) != 0) {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }

        return 1;

    }

    if (strcmp(_rts_transport_data.action, "message") == 0) { //聊天
        if (strlen(_rts_transport_data.token) == 0 || !_rts_transport_data.toid || !_rts_transport_data.id || wcslen(_rts_transport_data.content) == 0) {
            client_send(client, "{\"code\":\"0005\",\"message\":\"参数非法\"}");

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 0;
        }

        //认证信息失败，退出客户端
        if (!c_el->token || (strcmp(_rts_transport_data.token, "bsh_test_$%1KP@'") != 0 && strcmp(_rts_transport_data.token, c_el->token) != 0)) {
            client_send(client, "{\"code\":\"0004\",\"message\":\"token非法\"}");

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 0;
        }

        //判断登录者身份
        if (!c_el->id || c_el->id != _rts_transport_data.id) {
            client_send(client, "{\"code\":\"0006\",\"message\":\"用户身份非法\"}");

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 0;
        }

        bzero(key, 20);
        sprintf(key, "%d", _rts_transport_data.toid);
        client_id_elem *cid_el;
        ret = hashmap_get(client_id_map, key, (void **)&cid_el);
        

        printf("发给id:%s\n", key);
        if (ret == 0) {
            printf("get_toid:%s ret:%d fd:%d\n", key, ret, cid_el->fd);
        }
        if (ret == HMAP_E_NOTFOUND || !cid_el || !cid_el->fd) {
            client_send(client, "{\"code\":\"1002\",\"message\":\"对方不在线\"}");

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 1;
        }

        bzero(key, 20);
        sprintf(key, "%d", cid_el->fd);
        client_elem *to_c_el;
        ret = hashmap_get(client_map, key, (void **)&to_c_el);
        assert(ret == HMAP_S_OK);

        if (to_c_el->id == _rts_transport_data.id) { //如果接受者是自己，则发出警告
            client_send(client, "{\"code\":\"1001\",\"message\":\"不能给自己发消息\"}");

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 1;
        }

        bzero(buf, MAX_BUF + 1);
        sprintf(buf, "{\"code\":\"0000\",\"message\":\"发送成功\",\"content\":\"%ls\"}", _rts_transport_data.content);
        client_send(to_c_el->client, buf);

        /*解锁锁*/
        if (pthread_mutex_unlock(&mutex) != 0) {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }

        return 1;
    }

    if (strcmp(_rts_transport_data.action, "logout") == 0) { //logout
        if (strlen(_rts_transport_data.token) == 0 || !_rts_transport_data.id) {
            client_send(client, "{\"code\":\"0005\",\"message\":\"参数非法\"}");

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 0;
        }

        //判断登录者身份
        if (!c_el->id || c_el->id != _rts_transport_data.id) {
            client_send(client, "{\"code\":\"0006\",\"message\":\"用户身份非法\"}");

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 0;
        }

        //认证信息失败，退出客户端
        if (c_el->token && strcmp(_rts_transport_data.token, c_el->token) != 0) {
            client_send(client, "{\"code\":\"0004\",\"message\":\"token非法\"}");

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 0;
        }

        client_send(client, "{\"code\":\"0000\",\"message\":\"退出成功\"}");

        /*解锁锁*/
        if (pthread_mutex_unlock(&mutex) != 0) {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }

        return 0;
    }

    if (strcmp(_rts_transport_data.action, "register") == 0) {
        if (strlen(_rts_transport_data.name) == 0 || strlen(_rts_transport_data.password) == 0) {
            client_send(client, "{\"code\":\"0005\",\"message\":\"参数非法\"}");

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 0;
        }

        char *salt = RTS_rand();
        char *pwdhash = RTS_hash(_rts_transport_data.password, salt);
        char *datetime = RTS_current_datetime();
        _rts_transport_data.id = user_add(_rts_transport_data.name, pwdhash, salt, inet_ntoa(c_el->addr.sin_addr), datetime, 0);
        CLIENT_FREE(salt);
        CLIENT_FREE(pwdhash);
        CLIENT_FREE(datetime);

        if (_rts_transport_data.id == 0) {
            client_send(client, "{\"code\":\"1005\",\"message\":\"注册失败,该用户名已注册\"}");

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 1;
        }

        if (_rts_transport_data.id > 0) {
            bzero(buf, MAX_BUF + 1);
            sprintf(buf, "{\"code\":\"0000\",\"message\":\"注册成功\",\"id\":%d}", _rts_transport_data.id);
            client_send(client, buf);

            /*解锁锁*/
            if (pthread_mutex_unlock(&mutex) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }

            return 1;
        }

        client_send(client, "{\"code\":\"0007\",\"message\":\"注册失败,未知错误\"}");

        /*解锁锁*/
        if (pthread_mutex_unlock(&mutex) != 0) {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }

        return 1;
    }

    client_send(client, "{\"code\":\"0002\",\"message\":\"动作非法\"}");

    /*解锁锁*/
    if (pthread_mutex_unlock(&mutex) != 0) {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }

    return 0;
}