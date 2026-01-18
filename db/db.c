#include "db.h"
#include "stdio.h"
#include "string.h"
#include <stdlib.h>
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

    if (cfg && cfg->ssl_ca[0] !='\0'){
        const char *ssl_key = cfg->ssl_key[0] != '\0' ? cfg->ssl_key :NULL;
        const char *ssl_cert = cfg->ssl_cert[0] != '\0' ? cfg->ssl_cert : NULL;
        mysql_ssl_set(db->conn, ssl_key, ssl_cert, cfg->ssl_ca, NULL, NULL);
    }

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

    if (!db || !db->conn || !out_ts)
    return -1;

    *out_ts = (time_t)atoll(row[0]);

    mysql_free_result(res);
    return 0;
}

int db_insert_alarm(DBContext *db, int codigo, const char *descripcion)
{
    if (!db || !db->conn || !db->connected || !descripcion) {
        return -1;
    }

    const char *sql =
        "INSERT INTO alarmas_activas (codigo, descripcion, ts) "
        "VALUES (?, ?, NOW())";
    MYSQL_STMT *stmt = mysql_stmt_init(db->conn);
    if (!stmt) {
        snprintf(db->last_error, sizeof(db->last_error),
                 "mysql_stmt_init failed");
        return -1;
    }

    if (mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0) {
        snprintf(db->last_error, sizeof(db->last_error),
                 "mysql_stmt_prepare failed: %s",
                 mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));

    long codigo_param = (long)codigo;
    unsigned long desc_len = (unsigned long)strlen(descripcion);

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer      = (void *)&codigo_param;
    bind[0].is_null     = 0;

    bind[1].buffer_type   = MYSQL_TYPE_STRING;
    bind[1].buffer        = (void *)descripcion;
    bind[1].buffer_length = desc_len;
    bind[1].length        = &desc_len;
    bind[1].is_null       = 0;

    if (strlen(descripcion) >= 255) {
    snprintf(db->last_error, sizeof(db->last_error),
             "descripcion too long");
    return -1;
    }

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        snprintf(db->last_error, sizeof(db->last_error),
                 "mysql_stmt_bind_param failed: %s",
                 mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    if (mysql_stmt_execute(stmt) != 0) {
        snprintf(db->last_error, sizeof(db->last_error),
                 "mysql_stmt_execute failed: %s",
                 mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    mysql_stmt_close(stmt);
    return 0;
}

// Inserta un snapshot de máquina en la tabla machine_snapshots.
int db_insert_machine_snapshot(DBContext *db, const MachineSnapshot *snapshot)
{
    if (!db || !db->conn || !db->connected || !snapshot) {
        return -1;
    }

    const char *sql =
        "INSERT INTO machine_snapshots "
        "(state, total_units, good_units, reject_units, alarm_code, alarm_description, alarm_active, ts) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, NOW())";
    MYSQL_STMT *stmt = mysql_stmt_init(db->conn);
    if (!stmt) {
        snprintf(db->last_error, sizeof(db->last_error),
                 "mysql_stmt_init failed");
        return -1;
    }

    if (mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0) {
        snprintf(db->last_error, sizeof(db->last_error),
                 "mysql_stmt_prepare failed: %s",
                 mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    MYSQL_BIND bind[7];
    memset(bind, 0, sizeof(bind));

    unsigned char state_param = (unsigned char)snapshot->status.state;
    unsigned long total_param = (unsigned long)snapshot->counter.total_units;
    unsigned long good_param = (unsigned long)snapshot->counter.good_units;
    unsigned long reject_param = (unsigned long)snapshot->counter.reject_units;
    unsigned short alarm_code_param = (unsigned short)snapshot->alarm.alarm_code;
    unsigned char alarm_active_param = snapshot->alarm.active ? 1U : 0U;
    const char *alarm_desc_param = snapshot->alarm.description ? snapshot->alarm.description : "";
    unsigned long alarm_desc_len = (unsigned long)strlen(alarm_desc_param);

    if (alarm_desc_len >= 255) {
        snprintf(db->last_error, sizeof(db->last_error),
                 "alarm_description too long");
        mysql_stmt_close(stmt);
        return -1;
    }

    bind[0].buffer_type = MYSQL_TYPE_TINY;
    bind[0].buffer      = (void *)&state_param;
    bind[0].is_null     = 0;

    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer      = (void *)&total_param;
    bind[1].is_null     = 0;

    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer      = (void *)&good_param;
    bind[2].is_null     = 0;

    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer      = (void *)&reject_param;
    bind[3].is_null     = 0;

    bind[4].buffer_type = MYSQL_TYPE_SHORT;
    bind[4].buffer      = (void *)&alarm_code_param;
    bind[4].is_null     = 0;

    bind[5].buffer_type   = MYSQL_TYPE_STRING;
    bind[5].buffer        = (void *)alarm_desc_param;
    bind[5].buffer_length = alarm_desc_len;
    bind[5].length        = &alarm_desc_len;
    bind[5].is_null       = 0;

    bind[6].buffer_type = MYSQL_TYPE_TINY;
    bind[6].buffer      = (void *)&alarm_active_param;
    bind[6].is_null     = 0;

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        snprintf(db->last_error, sizeof(db->last_error),
                 "mysql_stmt_bind_param failed: %s",
                 mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    if (mysql_stmt_execute(stmt) != 0) {
        snprintf(db->last_error, sizeof(db->last_error),
                 "mysql_stmt_execute failed: %s",
                 mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    mysql_stmt_close(stmt);
    return 0;
}