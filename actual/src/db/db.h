#ifndef DB_H
#define DB_H
#include <mysql/mysql.h>
#include "config.h"

typedef struct{
    MYSQL *conn; //conexion con maria db  /////////////
    int connected; // 0=no, 1=si          ////////////////  DEBUG
    char last_error[256]; // ultimo error registrado ///
}DBContext;

int db_connect(DBContext *db, const DBConfig *cfg);

void db_close(DBContext *db);

int db_exec(DBContext *db, const char *sql);

int db_query_single_epoch(DBContext *db, const char *sql, time_t *out_ts);

int db_insert_alarm(DBContext *db, int codigo, const char *descripcion);


#endif // DB_H
