    /*//////////////Inicializa config
                    Inicializa DB
                    Inicializa OPC UA
                    Arranca loop principal
                    Maneja shutdown limpio//////////////////*/

#include <stdio.h>
#include "config.h"
#include "db.h"


int main(void){

    DBConfig cfg={0};
    DBContext db = {0};

    // carga valores para conexion a db 
    if (config_load("config.init", &cfg)!= 0){
        printf("error cargando config.ini\n");
        return 1;
    }
    // intenta conectar a db
    if (db_connect(&db, &cfg) != 0){
        printf("DB ERROR:%s\n", db.last_error);
        return 1;
    }
    // si conecta escribe este mensaje
    printf("DB CONEXION OK\n");

    while(1) {
        db_exec(&db,
        "INSERT INTO hearbeat(ts) VALUES (NOW())"
    );
    sleep(1);
    }
    //despues de conectar y pedir lo que necesita cierra
    if (db.conn){
        db_close(&db);
    }
    return 0;
}