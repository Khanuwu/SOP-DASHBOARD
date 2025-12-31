int db_connect(DBContext *db){
    if (db == NULL){
        return -1;
    }
}

db->conn = mysql_init(NULL); //crea el cliente en memoria 
if (db->conn == NULL){  //debug confirma si no tuvo error
    snprintf(db->last_error,sizeof(db->last_error),
            "mysql_init_failed"
            );
    db->connected = 0;
    return -1;
}

if(mysql_real_connect(   //inicio de socket tcp, handshake con servidor autenticacion y seleccion de db
                    db->conn,
                    "192.168.137.116", //inicio de socket tcp
                    "appuser", //autenticacion
                    "appuser$$2005", //autenticacion
                    "produccion", //seleccion de db
                    3306,
                    NULL,
                    0
                    )== NULL
){
    snprintf(db->last_error, sizeof(db->last_error), //debug para conexion con mariadb
            "mysql_real_connect failed:%s",
            mysql_error(db->conn)
            );

    mysql_close(db->conn);//si tiene error y no teniene conexiones cierra y limpia conexion
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
db->last_error[0] = "\0";
return0;