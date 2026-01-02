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


#endif // DB_H
