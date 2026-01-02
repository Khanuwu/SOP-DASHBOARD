#include "db.h"
#include "stdio.h"
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

    unsigned int t;

    t = cfg->connect_timeout;
    mysql_options(db->conn, MYSQL_OPT_CONNECT_TIMEOUT, &t);

    t = cfg->read_timeout;
    mysql_options(db->conn, MYSQL_OPT_READ_TIMEOUT, &t);

    t = cfg->write_timeout;
    mysql_options(db->conn, MYSQL_OPT_WRITE_TIMEOUT, &t);

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

    if(mysql_query(db->conn, "SELECT 1") != 0){
        snprintf(db->last_error, sizeof(db->last_error),
                "test query failes %s",
                mysql_error(db->conn)
                );

        mysql_close(db->conn);
        db->conn = NULL;
        db->connected = 0;
    return -1;
    }

    db->connected = 1;
    db->last_error[0] = '\0';
    return 0;
}


int db_exec(DBContext *db, const char *sql){
    if (!db || !db->conn || !sql){
        return -1;
    }

    if (mysql_query(db->conn, sql) != 0){
        snprintf(db->last_error, sizeof(db->last_error),
        "mysql_query failed: %s", mysql_error(db->conn));
    return -1;
    }
    return 0;
}
