#ifndef __RTSDB_H
#define __RTSDB_H

#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#define	DB_NAME				"RTServer.db"
#define	TABLE_NAME_USER		"user"
#define	TRUE				1
#define	FALSE				0

int open_db();
int modefiy_data();
int insert_data();
int search_data();

#endif