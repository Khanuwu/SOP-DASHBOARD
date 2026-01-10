#include "db.h"
#include "stdio.h"
#include "string.h"
#include <mysql/mysql.h>


void db_close(DBContext *db)
{
    if (!db || !db->conn) return;
    mysql_close(db->conn);
    db->conn = NULL;
    db->connected = 0;
}


int db_connect(DBContext *db, const DBConfig *cfg){
    if (db == NULL){
    return -1;
    }

    db->conn = mysql_init(NULL); //crea el cliente en memoria 
    if (db->conn == NULL){  //debug confirma si no tuvo error
        snprintf(db->last_error,sizeof(db->last_error),
                "mysql_init_failed"
                );
        db->connected = 0;
    return -1;
    }

    unsigned int connect_timeout =
    (cfg && cfg->connect_timeout > 0) ? (unsigned int)cfg->connect_timeout : 10;

    if (!mysql_real_connect(
            db->conn,
            cfg->host,
            cfg->user,
            cfg->password,
            cfg->database,
            cfg->port,
            NULL,
            0
    ))
    {
        snprintf(db->last_error, sizeof(db->last_error),
                "mysql_real_connect failed: %s",
                mysql_error(db->conn));

        mysql_close(db->conn);
        db->conn = NULL;
        db->connected = 0;
        return -1;
    }

    unsigned int read_timeout  =
    (cfg && cfg->read_timeout > 0) ? (unsigned int)cfg->read_timeout : 10;
    unsigned int write_timeout =
        (cfg && cfg->write_timeout > 0) ? (unsigned int)cfg->write_timeout : 10;

    if (mysql_options(db->conn, MYSQL_OPT_READ_TIMEOUT, &read_timeout) != 0) {
        snprintf(db->last_error, sizeof(db->last_error),
                "mysql_options READ_TIMEOUT failed");
    // no cierres; decide si quieres seguir sin timeout
    }

    if (mysql_options(db->conn, MYSQL_OPT_WRITE_TIMEOUT, &write_timeout) != 0) {
        snprintf(db->last_error, sizeof(db->last_error),
                "mysql_options WRITE_TIMEOUT failed");
        // no cierres; decide si quieres seguir sin timeout
    }

    if (mysql_ping(db->conn) != 0){ //hace un ping hacia la db 
        snprintf(db->last_error, sizeof(db->last_error), //debug por si falla   
                "mysql_ping failed :%s",
                mysql_error(db->conn)
                );

        mysql_close(db->conn);
        db->conn = NULL; //cierra conexion y limpia conn para conectar
        db->connected =0;
    return -1;
    }

    db->connected = 1;
    db->last_error[0] = '\0';
    return 0;
}


int db_exec(DBContext *db, const char *sql)
{
    if (!db || !db->conn || !sql) {
        return -1;
    }

    if (mysql_query(db->conn, sql) != 0) {
        snprintf(db->last_error, sizeof(db->last_error),
                 "mysql_query failed: %s",
                 mysql_error(db->conn));
        return -1;
    }

    /* CLAVE: limpiar resultados pendientes */
    MYSQL_RES *res = mysql_store_result(db->conn);
    if (res) {
        mysql_free_result(res);
    }

    return 0;
}

int db_query_single_epoch(DBContext *db, const char *sql, time_t *out_ts)
{
    MYSQL_RES *res;
    MYSQL_ROW row;

    if (mysql_query(db->conn, sql) != 0)
        return -1;

    res = mysql_store_result(db->conn);
    if (!res)
        return -1;

    row = mysql_fetch_row(res);
    if (!row || !row[0]) {
        mysql_free_result(res);
        return -1;
    }

    *out_ts = (time_t)atoll(row[0]);

    mysql_free_result(res);
    return 0;
}
