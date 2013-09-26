/*
apt-get install libsqlite3-dev
gcc -o manage manage.c data.c ../tools/base.c ../md5/md5.c -lsqlite3
*/

#include <stdio.h>
#include "data.h"
#include "../tools/base.h"

/**
* 数据的添加
*/
int insert_data() {
	int flag = 1;
	char name[20], password[32], salt[6], ip[15], *datetime;
	int rid;
	while (flag == 1)	{		
		getchar();
		printf("name:");
		scanf("%s", name);
		printf("password:");
		scanf("%s", password);
		printf("ip:");
		scanf("%s", ip);

		printf("%s\n", name);
		memset(salt, 0, 6);
		sprintf(salt, "%s", RTS_rand());
		printf("%s\n", name);
		strcpy(password, RTS_hash(password, salt));
		printf("%s\n", name);
		strcpy(datetime, "abcd");
		datetime = RTS_current_datetime();
		free(datetime);
		datetime = NULL;
		//printf("%s\n", name);

		rid = user_add(name, password, salt, ip, datetime, 0);
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

int main(int argc, char **argv) {
	int n;
	printf("请输入代表数字：0.建表 1.添加 2.修改 3.删除 4.查看\n");
	printf("choose[0 - 4]:");
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
			printf("修改成功\n");
			break;
		case 3:
			printf("删除成功\n");
			break;
		case 4:
			search_data();
			printf("查看成功\n");
			break;
		default :
			printf("\nerror\n");
	}
	return 0;
}
