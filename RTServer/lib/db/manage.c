/*
apt-get install libsqlite3-dev
gcc -o manage manage.c data.c -lsqlite3
*/

#include <stdio.h>
#include "data.h"

int main(int argc , char **argv)
{
	int n;
	printf("\n 1. 添加 2.修改 3.删除  4.查看\n ");
	creat_table();
	printf("choose[1 - 4]:");
	scanf("%d" , &n);
	switch(n)
	{
		case 1:
			
			insert_data();
		//	printf("添加成功\n");
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
