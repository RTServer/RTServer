#include "data.h"

sqlite3 *pdb = NULL ;
char *szErrMsg = 0 ;
/*
*@brief //打开数据库
*@param  pdb 类型为sqlite 
*/
int open_db()
{
	int rc;
	rc = sqlite3_open(DB_NAME , &pdb);
	if(rc)
	{
		fprintf(stderr , "can't open database: %s\n",sqlite3_errmsg(pdb));
		sqlite3_close(pdb) ;
		return -1;
	}
	return 0;
}



//@brif 数据库语句执行


int creat_table()
{
	int rc;
	char *strsql = "create table user(sid varchar(10) not null primary key , sname varchar(10)  not null , time Datetime )";
	if(open_db != 0)
	{
		open_db();
	}
	rc = sqlite3_exec(pdb , strsql , 0 , 0 ,&szErrMsg);
	if(rc != 0)
	{
		fprintf(stderr , "can't open database: %s\n",sqlite3_errmsg(pdb));
		sqlite3_close(pdb);
		return -1;
	}	
	return 0;
}

int modefiy_data()
{
	if(open_db != 0)
	{
		open_db();
	}
	return 0;
}
/*
*数据的添加
*/

int insert_data()		
{
	int flag = TRUE;
	char buff[1024];
	char *strsql;
	int rc = 0 ;
	char temp[3][20];
	strsql = buff;
	if(open_db != 0)
	{
		open_db();
	}
	strcpy(strsql , "insert into ");
	strcat(strsql , TABLE_NAME_USER);
	strcat(strsql , " values(? , ? , ?);");

	sqlite3_stmt  *stmt = NULL;
	rc = sqlite3_prepare_v2(pdb , strsql , strlen(strsql) , &stmt , NULL);
	if(rc != SQLITE_OK)
	{
		if(stmt)
		{
			sqlite3_finalize(stmt);
		}
		sqlite3_close(pdb);
		return -1;
	}

	while(flag)	
	{		
			getchar();
			memset(temp,0,sizeof(temp));
			printf("学号：");
			scanf("%s", temp[0]);
			printf("姓名：");
			scanf("%s", temp[1]);
			printf("性别：");
			scanf("%s", temp[2]);

			//getchar();
			sqlite3_bind_text( stmt , 1 , temp[0] , strlen(temp[0]), NULL);
			
			sqlite3_bind_text( stmt , 2 , temp[1] , strlen(temp[1]), NULL);
			
			sqlite3_bind_text( stmt , 3 , temp[2] , strlen(temp[2]), NULL);
			if(sqlite3_step(stmt) != SQLITE_DONE)
			{	
				
				sqlite3_finalize(stmt);
				sqlite3_close(pdb);
				printf(" faile \n" );
				return 0;
			}		
		sqlite3_reset(stmt);
		printf("Do you want to do insert ?[0 ~ 1]:");
		scanf("%d", &flag);
	}
	sqlite3_finalize(stmt);
	printf("insert success!\n");
	sqlite3_close(pdb);
	
	
	return 0;
	
}
/*
*	数据的查询
*/
int search_data()
{
	int rc;
	if(open_db != 0)
	{
		open_db();
	}
	char *strsql = "select * from user" ;

	sqlite3_stmt  *stmt = NULL;

	rc = sqlite3_prepare_v2(pdb , strsql , strlen(strsql) , &stmt , NULL);
	if(rc != SQLITE_OK)
	{
		if(stmt)
		{
			sqlite3_finalize(stmt);
		}
		sqlite3_close(pdb);
		return -1;
	}
	int nColumn = sqlite3_column_count(stmt);
	int vtype , i;
	do{	
		rc = sqlite3_step(stmt);
		if(rc == SQLITE_ROW)
		{
			
			for(i = 0 ; i < nColumn ; i++ )
			{
			
				vtype = sqlite3_column_type(stmt , i);
				if(vtype == SQLITE_INTEGER)
				{
					printf("%s : %d \n" , sqlite3_column_name(stmt , i) , sqlite3_column_int(stmt , i));
				}
				else if(vtype == SQLITE_TEXT)
				{
					printf("%s : %s \n" , sqlite3_column_name(stmt , i) , sqlite3_column_text(stmt , i));
				}
				else if(vtype == SQLITE_NULL)
				{
					printf("no values\n");
				}
			}
			printf("\n****************\n");
			
		}
		else if(rc == SQLITE_DONE)
		{
			printf("Select finish\n");
			break;
		}
		else
		{
			printf("Select faile\n");
			sqlite3_finalize(stmt);
			break;
			
		}
	
		
	}while(1);
	sqlite3_finalize(stmt);
	sqlite3_close(pdb);
	return 0;
}