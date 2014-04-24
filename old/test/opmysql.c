/**
 * gcc -o opmysql opmysql.c `/server/mysql/bin/mysql_config --cflags --libs`
 */

#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    MYSQL *connPtr;
    MYSQL_RES *resPtr;
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    int res, i, j;

    connPtr = mysql_init(NULL);
    if(!connPtr) {
        fprintf(stderr, "mysql_init failedn");
        return EXIT_FAILURE;
    }
    connPtr = mysql_real_connect(connPtr, "211.88.253.8", "importmall", "123456", "importmall", 3306, NULL, 0);
    if(connPtr) {
        //printf("Connection successn");
        mysql_query(connPtr, "SET NAMES utf8");

        res = mysql_query(connPtr, "SELECT * FROM ecs_ad"); //查询语句
        //res = mysql_query(connPtr, "INSERT INTO t VALUES(null,'Ann')"); //可以把insert语句替换成delete或者update语句
        if(res) {
            fprintf(stderr, "Insert error %d: %sn", mysql_errno(connPtr), mysql_error(connPtr));
        }else {
            //printf("Inserted %lu rowsn", (unsigned long)mysql_affected_rows(connPtr)); //输出受影响的行数

            resPtr = mysql_store_result(connPtr); //取出结果集
            if(resPtr) {
                printf("%lu Rowsn", (unsigned long)mysql_num_rows(resPtr));
                field = mysql_fetch_fields(resPtr);
                j = mysql_num_fields(resPtr);
                for(i = 0; i < j; i++) {
                    printf("%st", field[i].name);
                }
                printf("n");
                while(row = mysql_fetch_row(resPtr)) { //依次取出记录
                    for(i = 0; i < j; i++) {
                        printf("%st", row[i]);
                    }
                    printf("n");
                }
                if(mysql_errno(connPtr)) {
                    fprintf(stderr,"Retrive error:sn", mysql_error(connPtr));
                }
            }
            mysql_free_result(resPtr);
        }
    } else {  
        printf("Connection failedn");
    }
    mysql_close(connPtr);
    return EXIT_SUCCESS;
}