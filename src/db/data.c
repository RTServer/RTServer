#include <stdio.h>
#include <stdlib.h>
#include "data.h"

sqlite3 *pdb = NULL ;
char *szErrMsg = 0 ;

/**
* 初始化一个用户结构
*/
_RTS_USER user_init() {
	static _RTS_USER _rts_user;
    _rts_user.id = -1;
    memset(_rts_user.name, 0, 21);
    memset(_rts_user.password, 0, 33);
    memset(_rts_user.salt, 0, 7);
    memset(_rts_user.ip, 0, 16);
    memset(_rts_user.datetime, 0, 20);
    _rts_user.status = -1;
    return _rts_user;
}

/**
 * 打开数据库
 * pdb 类型为sqlite
 */
int open_db() {
	int rc;
	sqlite3_config(SQLITE_CONFIG_SERIALIZED); //设置串行，线程安全
	rc = sqlite3_open(DB_NAME, &pdb);
	if (rc) {
		fprintf(stderr, "can't open database: %s\n", sqlite3_errmsg(pdb));
		sqlite3_close(pdb);
		return -1;
	}
	return 0;
}


/**
* 创建表结构
*/
int creat_table() {
	int rc;
	char strsql[1025];
	memset(strsql, 0, 1025);
	strcpy(strsql, "CREATE TABLE ");
	strcat(strsql, TABLE_NAME_USER);
	strcat(strsql, "(id INTEGER PRIMARY KEY AUTOINCREMENT, name VARCHAR(20) NOT NULL DEFAULT '', password VARCHAR(32) NOT NULL DEFAULT '', salt VARCHAR(6) NOT NULL DEFAULT '', ip VARCHAR(15) NOT NULL DEFAULT '', datetime DATETIME DEFAULT '', status BOOLEAN DEFAULT 0)");
	if (open_db() == -1) return -1;
	rc = sqlite3_exec(pdb, strsql, 0, 0, &szErrMsg);
	if (rc != 0) {
		fprintf(stderr, "can't open database: %s\n", sqlite3_errmsg(pdb));
		sqlite3_close(pdb);
		return -1;
	}	
	return 0;
}

/**
* 添加用户
*/
int user_add(char *name, char *password, char *salt,
	char *ip, char *datetime, int status) {
	int rc = 0 ;
	char strsql[1025];
	
	if (open_db() == -1) return -1;

	sqlite3_stmt *stmt = NULL;

	//判断是否已存在该用户名，如果存在返回0
	memset(strsql, 0, 1025);
	strcpy(strsql, "SELECT id FROM ");
	strcat(strsql, TABLE_NAME_USER);
	strcat(strsql, " WHERE name = ?");
	rc = sqlite3_prepare_v2(pdb, strsql, strlen(strsql), &stmt, NULL);
	if (rc != SQLITE_OK) {
		if (stmt) {
			sqlite3_finalize(stmt);
		}
		sqlite3_close(pdb);
		return -1;
	}
	sqlite3_bind_text(stmt, 1, name, strlen(name), NULL);
	int nColumn = sqlite3_column_count(stmt);
	int vtype, i, rid = 0;
	do {	
		rc = sqlite3_step(stmt);
		if (rc == SQLITE_ROW) {
			for(i = 0; i < nColumn; i++) {
				vtype = sqlite3_column_type(stmt, i);
				if (vtype == SQLITE_INTEGER) {
					rid = sqlite3_column_int(stmt, i);
				} else if (vtype == SQLITE_TEXT) {
					
				} else if (vtype == SQLITE_NULL) {
					
				}
			}
		} else if (rc == SQLITE_DONE) {
			//printf("Select finish\n");
			break;
		} else {
			//printf("Select faile\n");
			sqlite3_finalize(stmt);
			break;
		}
	} while(1);
	sqlite3_finalize(stmt);
	if (rid != 0) {
		return 0;
	}

	memset(strsql, 0, 1025);
	strcpy(strsql, "INSERT INTO ");
	strcat(strsql, TABLE_NAME_USER);
	strcat(strsql, " VALUES(NULL, ?, ?, ?, ?, ?, ?)");

	stmt = NULL;
	rc = sqlite3_prepare_v2(pdb, strsql, strlen(strsql), &stmt, NULL);
	if (rc != SQLITE_OK) {
		if (stmt) {
			sqlite3_finalize(stmt);
		}
		sqlite3_close(pdb);
		return -1;
	}
	sqlite3_bind_text(stmt, 1, name, strlen(name), NULL);	
	sqlite3_bind_text(stmt, 2, password, strlen(password), NULL);
	sqlite3_bind_text(stmt, 3, salt, strlen(salt), NULL);
	sqlite3_bind_text(stmt, 4, ip, strlen(ip), NULL);
	sqlite3_bind_text(stmt, 5, datetime, strlen(datetime), NULL);
	sqlite3_bind_int(stmt, 6, status);
	if (sqlite3_step(stmt) != SQLITE_DONE) {	
		sqlite3_finalize(stmt);
		sqlite3_close(pdb);
		return -1;
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	int id = sqlite3_last_insert_rowid(pdb);
	sqlite3_close(pdb);

	return id;
}

/**
* 获取用户信息
*/
_RTS_USER *user_get(int id, char *name) {
	_RTS_USER *_rts_user = NULL;
	int rc = 0 ;
	char strsql[1025];
	
	if (open_db() == -1) return _rts_user;

	sqlite3_stmt *stmt = NULL;

	memset(strsql, 0, 1025);
	strcpy(strsql, "SELECT * FROM ");
	strcat(strsql, TABLE_NAME_USER);
	id && strcat(strsql, " WHERE id = ?");
	id || strcat(strsql, " WHERE name = ?");
	rc = sqlite3_prepare_v2(pdb, strsql, strlen(strsql), &stmt, NULL);
	if (rc != SQLITE_OK) {
		if (stmt) {
			sqlite3_finalize(stmt);
		}
		sqlite3_close(pdb);
		return _rts_user;
	}

	//申请空间
	_rts_user = (_RTS_USER *)malloc(sizeof(_RTS_USER));
	if (_rts_user == NULL) return _rts_user;

	id && sqlite3_bind_int(stmt, 1, id);
	id || sqlite3_bind_text(stmt, 1, name, strlen(name), NULL);
	int nColumn = sqlite3_column_count(stmt);
	int i;
	do {	
		rc = sqlite3_step(stmt);
		if (rc == SQLITE_ROW) {
			for(i = 0; i < nColumn; i++) {
				const char *colum_name = sqlite3_column_name(stmt, i);
				(strcmp(colum_name, "id") == 0) && (_rts_user->id = sqlite3_column_int(stmt, i));
				(strcmp(colum_name, "name") == 0) && strcpy(_rts_user->name, (char *)sqlite3_column_text(stmt, i));
				(strcmp(colum_name, "password") == 0) && strcpy(_rts_user->password, (char *)sqlite3_column_text(stmt, i));
				(strcmp(colum_name, "salt") == 0) && strcpy(_rts_user->salt, (char *)sqlite3_column_text(stmt, i));
				(strcmp(colum_name, "ip") == 0) && strcpy(_rts_user->ip, (char *)sqlite3_column_text(stmt, i));
				(strcmp(colum_name, "datetime") == 0) && strcpy(_rts_user->datetime, (char *)sqlite3_column_text(stmt, i));
				(strcmp(colum_name, "status") == 0) && (_rts_user->status = sqlite3_column_int(stmt, i));
			}
		} else if (rc == SQLITE_DONE) {
			//printf("Select finish\n");
			break;
		} else {
			//printf("Select faile\n");
			sqlite3_finalize(stmt);
			break;
		}
	} while(1);
	sqlite3_finalize(stmt);
	sqlite3_close(pdb);
	return _rts_user;
}

/**
* 修改用户表
*/
int user_edit(_RTS_USER _rts_user) {
	int rc = 0 ;
	char strsql[1025], tmp[1025];
	int len = 0, sqllength = 0;

	if (_rts_user.id <= 0) return -1;
	
	if (open_db() == -1) return -1;

	memset(strsql, 0, 1025);
	memset(tmp, 0, 1025);

	sprintf(tmp, "UPDATE %s SET ", TABLE_NAME_USER);
	len = strlen(tmp);
	sqllength += len;
	if (sqllength > 1024) return -1;
	strncat(strsql, tmp, len);

	if (strlen(_rts_user.password) > 0) {
		sprintf(tmp, "password='%s',", _rts_user.password);
		len = strlen(tmp);
		sqllength += len;
		if (sqllength > 1024) return -1;
		strncat(strsql, tmp, len);
	}
	if (strlen(_rts_user.salt) > 0) {
		sprintf(tmp, "salt='%s',", _rts_user.salt);
		len = strlen(tmp);
		sqllength += len;
		if (sqllength > 1024) return -1;
		strncat(strsql, tmp, len);
	}
	if (strlen(_rts_user.ip) > 0) {
		sprintf(tmp, "ip='%s',", _rts_user.ip);
		len = strlen(tmp);
		sqllength += len;
		if (sqllength > 1024) return -1;
		strncat(strsql, tmp, len);
	}
	if (strlen(_rts_user.datetime) > 0) {
		sprintf(tmp, "datetime='%s',", _rts_user.datetime);
		len = strlen(tmp);
		sqllength += len;
		if (sqllength > 1024) return -1;
		strncat(strsql, tmp, len);
	}
	if (_rts_user.status == 1 || _rts_user.status == 0) {
		sprintf(tmp, "status=%d,", _rts_user.status);
		len = strlen(tmp);
		sqllength += len;
		if (sqllength > 1024) return -1;
		strncat(strsql, tmp, len);
	}

	strsql[sqllength - 1] = '\0';
	sprintf(tmp, " WHERE id=%d", _rts_user.id);
	len = strlen(tmp);
	sqllength += len;
	if (sqllength > 1024) return -1;
	strncat(strsql, tmp, len);

	rc = sqlite3_exec(pdb, strsql, 0, 0, &szErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "can't open database: %s\n", sqlite3_errmsg(pdb));
		return -1;
	}
	sqlite3_close(pdb);
	return 0;
}

/**
* 删除用户
*/
int user_del(int id) {
	int rc = 0 ;
	char strsql[1025];

	if (id <= 0) return -1;
	
	if (open_db() == -1) return -1;

	memset(strsql, 0, 1025);

	sprintf(strsql, "DELETE FROM %s WHERE id=%d", TABLE_NAME_USER, id);

	rc = sqlite3_exec(pdb, strsql, 0, 0, &szErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "can't open database: %s\n", sqlite3_errmsg(pdb));
		return -1;
	}
	sqlite3_close(pdb);
	return 0;
}

/**
* 数据的查询
*/
int search_data() {
	int rc = 0;
	if (open_db() == -1) return -1;
	char strsql[1024];
	memset(strsql, 0, 1024);
	strcpy(strsql, "SELECT * FROM ");
	strcat(strsql, TABLE_NAME_USER);
	//strcat(strsql, " WHERE status = 1");

	sqlite3_stmt  *stmt = NULL;

	rc = sqlite3_prepare_v2(pdb , strsql , strlen(strsql) , &stmt , NULL);
	if (rc != SQLITE_OK) {
		if(stmt) {
			sqlite3_finalize(stmt);
		}
		sqlite3_close(pdb);
		return -1;
	}
	int nColumn = sqlite3_column_count(stmt);
	int vtype, i;
	do {	
		rc = sqlite3_step(stmt);
		if (rc == SQLITE_ROW) {
			printf("****************\n");
			for(i = 0; i < nColumn; i++) {
				vtype = sqlite3_column_type(stmt, i);
				if (vtype == SQLITE_INTEGER) {
					printf("%s : %d \n", sqlite3_column_name(stmt, i), sqlite3_column_int(stmt, i));
				} else if (vtype == SQLITE_TEXT) {
					printf("%s : %s \n", sqlite3_column_name(stmt, i), sqlite3_column_text(stmt, i));
				} else if (vtype == SQLITE_NULL) {
					printf("no values\n");
				}
			}
			printf("****************\n\n");
		} else if (rc == SQLITE_DONE) {
			printf("Select finish\n");
			break;
		} else {
			printf("Select faile\n");
			sqlite3_finalize(stmt);
			break;
		}
	
		
	} while(1);
	sqlite3_finalize(stmt);
	sqlite3_close(pdb);
	return 0;
}