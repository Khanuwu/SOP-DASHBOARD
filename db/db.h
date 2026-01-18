#ifndef DB_H
#define DB_H
#include <mysql/mysql.h>
#include "config.h"
#include "machine/machine.h"

typedef struct{
    MYSQL *conn; //conexion con maria db  /////////////
    int connected; // 0=no, 1=si          ////////////////  DEBUG
    char last_error[256]; // ultimo error registrado ///
}DBContext;

typedef enum {
    DB_OK = 0,
    DB_ERR_PARAM = -1,
    DB_ERR_STMT_INIT = -2,
    DB_ERR_STMT_PREPARE = -3,
    DB_ERR_STMT_BIND = -4,
    DB_ERR_STMT_EXEC = -5
} DBResult;


int db_connect(DBContext *db, const DBConfig *cfg);

void db_close(DBContext *db);

int db_exec(DBContext *db, const char *sql);

int db_query_single_epoch(DBContext *db, const char *sql, time_t *out_ts);

int db_insert_alarm(DBContext *db, int codigo, const char *descripcion);

int db_insert_machine_snapshot(DBContext *db, const MachineSnapshot *snapshot);


#endif // DB_H
