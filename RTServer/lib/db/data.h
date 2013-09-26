#ifndef __RTSDB_H
#define __RTSDB_H

#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#define	DB_NAME				"RTServer.db"
#define	TABLE_NAME_USER		"user"

extern int open_db();
extern int creat_table();
extern int user_add(char *name, char *password, char *salt,
	char *ip, char *datetime, int status);
extern int insert_data();
extern int search_data();

#endif