    /*//////////////Inicializa config
                    Inicializa DB
                    Inicializa OPC UA
                    Arranca loop principal
                    Maneja shutdown limpio//////////////////*/

#include <stdio.h>
#include "config.h"
#include "db.h"
#include "opcua/opcua_client.h"
#include <windows.h>
#include <time.h>

//funcion auxiliar para pasar sql en tiempo 

time_t parse_mysql_ts(const char *ts){
    struct tm tm = {0};

    //formato Mysql= YYYY-MM-DD HH:MM:SS
    if(sscanf(ts,"%d-%d-%d %d:%d:%d",
        &tm.tm_year,
        &tm.tm_mon,
        &tm.tm_mday,
        &tm.tm_hour,
        &tm.tm_min,
        &tm.tm_sec) !=6){
            return(time_t)-1;
        }

        tm.tm_year -= 1900;
        tm.tm_mon -= 1;

        return mktime(&tm);
    }

// funcion principal main

int main(void)
{
    DBConfig cfg = {0};
    DBContext db  = {0};

    time_t hb_age_sec = 0;
    double diff_sec;
    const int opcua_read_interval_sec = 5;
    time_t opcua_last_read = 0;

    /* 1) Cargar configuración */
    if (config_load("config.init", &cfg) != 0) {
        printf("ERROR cargando config.ini\n");
        return 1;
    }

    /* 2) Conectar a la base de datos */
    if (db_connect(&db, &cfg) != 0) {
        printf("DB ERROR: %s\n", db.last_error);
        return 1;
    }

    printf("DB CONEXION OK\n");

    /* 3) Loop principal */
    while (1) {

        time_t now = time(NULL);

        /* 3.0 Leer OPC UA cada N segundos */
        if(now!= (time_t)-1 &&
           difftime(now, opcua_last_read) >= opcua_read_interval_sec){
            UA_StatusCode opcua_status = opcua_client_read_temperature();

            if(opcua_status != UA_STATUSCODE_GOOD){
                printf("OPC UA READ ERROR: 0x%08x\n", opcua_status);
            }
            opcua_last_read = now;
        }

        /* 3.1 Actualiza heartbeat propio (este proceso) */
        if (db_exec(&db,
            "UPDATE hearbeat SET ts = NOW() WHERE id = 1") != 0)
        {
            printf("DB EXEC ERROR: %s\n", db.last_error);

            db_close(&db);
            Sleep(1000);

            if (db_connect(&db, &cfg) != 0) {
                printf("RECONNECT FAILED: %s\n", db.last_error);
                Sleep(5000);
            }
            continue;
        }

        /* 3.2 Lee edad del heartbeat desde DB (segundos) */
        if (db_query_single_epoch(
                &db,
                "SELECT TIMESTAMPDIFF(SECOND, ts, NOW()) FROM hearbeat WHERE id = 1",
                &hb_age_sec
            ) != 0)
        {
            printf("READ HEARTBEAT FAILED\n");
            Sleep(1000);
            continue;
        }

        /* 3.3 Calcula edad del heartbeat */
        diff_sec = (double)hb_age_sec;

        printf("Heartbeat age: %.0f sec\n", diff_sec);

        /* 3.4 Evaluación de alarma */
        if (diff_sec > 3) {
            printf("ALARM: HEARTBEAT TIMEOUT\n");

            db_insert_alarm(&db, 1001, "Heartbeat perdido");
        }

        Sleep(1000);
    }

    /* Nunca llega aquí, pero queda correcto */
    if (db.conn) {
        db_close(&db);
    }

    return 0;
}