/*///////////////////////////////////////////////////////////
    Inicializa config
    Inicializa DB
    Arranca loop principal
    Maneja heartbeat y alarma con latch
///////////////////////////////////////////////////////////*/

#include <stdio.h>
#include "config.h"
#include "db.h"
#include <windows.h>
#include <time.h>

int main(void)
{
    DBConfig  cfg = {0};
    DBContext db  = {0};

    time_t hb_age_sec = 0;   // edad del heartbeat en segundos (desde DB)
    int  alarm_active = 0; // latch de alarma (0 = no activa, 1 = activa)

    /*--------------------------------------------------------
     1) Cargar configuración
    --------------------------------------------------------*/
    if (config_load("config.init", &cfg) != 0) {
        printf("ERROR cargando config.ini\n");
        return 1;
    }

    /*--------------------------------------------------------
     2) Conectar a la base de datos
    --------------------------------------------------------*/
    if (db_connect(&db, &cfg) != 0) {
        printf("DB ERROR: %s\n", db.last_error);
        return 1;
    }

    printf("DB CONEXION OK\n");

    /*--------------------------------------------------------
     3) Loop principal
    --------------------------------------------------------*/
    while (1) {

        /*----------------------------------------------------
         3.1 Actualiza heartbeat propio
         (este proceso mantiene vivo su ID)
        ----------------------------------------------------*/
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

        /*----------------------------------------------------
         3.2 Lee edad del heartbeat directamente desde MariaDB
         (la DB es la fuente de tiempo)
        ----------------------------------------------------*/
        if (db_query_single_epoch(
                &db,
                "SELECT TIMESTAMPDIFF(SECOND, ts, NOW()) "
                "FROM hearbeat WHERE id = 1",
                &hb_age_sec
            ) != 0)
        {
            printf("READ HEARTBEAT FAILED\n");
            Sleep(1000);
            continue;
        }


        /*----------------------------------------------------
         3.3 Evaluación de alarma (con latch)
        ----------------------------------------------------*/
        if (hb_age_sec > 3 && !alarm_active) {

            printf("ALARM: HEARTBEAT TIMEOUT\n");
            alarm_active = 1;

            db_exec(&db,
                "INSERT INTO alarmas_activas (codigo, descripcion, ts) "
                "VALUES (1001, 'Heartbeat perdido', NOW())"
            );
        }

        if (hb_age_sec <= 3 && alarm_active) {

            printf("HEARTBEAT RECOVERED\n");
            alarm_active = 0;

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
