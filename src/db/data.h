#ifndef __RTSDB_H
#define __RTSDB_H

#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#define	DB_NAME				"objs/db/RTServer.db"
#define	TABLE_NAME_USER		"user"

//定义用户数据结构
typedef struct _RTS_USER {
	int id;
	char name[21];
	char password[33];
	char salt[7];
	char ip[16];
	char datetime[20];
	int status;
}_RTS_USER;

extern _RTS_USER user_init();
extern int open_db();
extern int creat_table();
extern int user_add(char *name, char *password, char *salt,
	char *ip, char *datetime, int status);
extern _RTS_USER user_get(int id, char *name);
extern int user_edit(_RTS_USER _rts_user);
extern int user_del(int id);
extern int insert_data();
extern int search_data();

#endif