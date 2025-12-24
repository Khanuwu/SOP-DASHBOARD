#include "db.h"
#include "stdio.h"
#include "stdlib.h"

#define DB_HOST "localhost"
#define DB_USER "appuser"
#define DB_PASS "appuser$$2005"
#define DB_NAME "produccion"

MYSQL *db_connect(void){
    MYSQL *conn = mysql_init(NULL);

    if(conn == NULL){
        fprintf(stderr, "Error mysql_init()\n");
        return NULL;
    }

    if (mysql_real_connect(
        conn,
        DB_HOST,
        DB_USER,
        DB_PASS,
        DB_NAME,
        0,
        NULL,
        0)== NULL){
            fprintf(stderr,"Error mysql_real_connect(): %s\n",
            mysql_error(conn));
        mysql_close(conn);
        retunr NULL;
        }

        mysql_set_character_set(conn, "utf8mb4");

        return conn;
}

void db_close(MYSQL *conn){
    if(conn != NULL){
        mysql_close(conn);
    }
}