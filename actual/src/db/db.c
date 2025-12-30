/*//////////////Insert / Update
                Reintentos
                Pool de conexiones
                Transacciones//////////////////*/

MariadbTestConexion(){
    
    MYSQL *conn = mysql_init(NULL); // crea el objeto de conexion memoria

    mysql_real_connect(
        conn,  //parametros para conexion conn
        "192.168.137.166", //host
        "appuser", // usuario
        "appuser$$2005", // contraseña
        "produccion", // db
        3306, // puerto
        NULL, 
        0
    );

    mysql_ping(conn); // revisa conexion si esta okey
    mysql_query(conn, "SELECT 1"); // intenta una seleccion para probar
    mysql_close(conn); //cierra conexion
}
                    