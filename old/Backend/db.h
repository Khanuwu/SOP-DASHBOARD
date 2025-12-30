#ifndef DB_H
#define DB_H

#include <mariadb/mysql.h>

MYSQL *db_connect(void);

void db_close(MYSQL *conn);

#endif

