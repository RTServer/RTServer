/*
apt-get install libsqlite3-dev
gcc -o RTSAdmin RTSAdmin.c lib/db/data.c lib/tools/base.c lib/md5/md5.c -lsqlite3
*/

#include <stdio.h>
#include <stdlib.h>
#include "../db/data.h"
#include "../tools/base.h"

/**
* 数据的添加
*/
int insert_data() {
	int flag = 1;
	char name[21], password[33], *pwdhash, *salt, ip[16], *datetime;
	int rid;
	while (flag == 1)	{
		getchar();
		printf("name:");
		scanf("%s", name);
		printf("password:");
		scanf("%s", password);
		printf("ip:");
		scanf("%s", ip);

		salt = RTS_rand();
		pwdhash = RTS_hash(password, salt);
		datetime = RTS_current_datetime();

		rid = user_add(name, pwdhash, salt, ip, datetime, 0);
		free(salt); salt = NULL;
		free(pwdhash); pwdhash = NULL;
		free(datetime); datetime = NULL;

		if (rid == 0) {
			printf("have\n");
		} else if (rid > 0) {
			printf("ok-%d\n", rid);
		} else {
			printf("error\n");
		}

		printf("Do you want to do insert continue ?[0 ~ 1]:");
		scanf("%d", &flag);
	}

	return 0;
}

void user_info(int id, char *name) {
	_RTS_USER _rts_user = user_get(id, name);
	printf("****************\n");
	printf("id : %d \n", _rts_user.id);
	printf("name : %s \n", _rts_user.name);
	printf("password : %s \n", _rts_user.password);
	printf("salt : %s \n", _rts_user.salt);
	printf("datetime : %s \n", _rts_user.datetime);
	printf("status : %d \n", _rts_user.status);
	printf("****************\n\n");
}

int main(int argc, char **argv) {
	int n, id;
	char name[21];
	printf("请输入代表数字：\n0.建表\n1.添加\n2.查看全部\n3.根据id查看\n4.根据name查看\n5.删除\n");
	printf("choose[0 - 5]:");
	scanf("%d", &n);
	switch(n) {
		case 0:
			creat_table();
			printf("建表成功\n");
			break;
		case 1:
			insert_data();
			printf("添加成功\n");
			break;
		case 2:
			search_data();
			printf("查看成功\n");
			break;
		case 3:
			printf("请输入id:");
			scanf("%d", &id);
			user_info(id, NULL);
			printf("查看成功\n");
			break;
		case 4:
			printf("请输入name:");
			scanf("%s", name);
			user_info(0, name);
			printf("查看成功\n");
			break;
		case 5:
			printf("请输入id:");
			scanf("%d", &id);
			user_del(id);
			printf("删除成功\n");
			break;
		default :
			printf("\nerror\n");
	}
	return 0;
}
