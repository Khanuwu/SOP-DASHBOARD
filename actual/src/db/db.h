typedef struct{
    MYSQL *conn; //conexion con maria db  /////////////
    int connected; // 0=no, 1=si          ////////////////  DEBUG
    char last_error[256] // ultimo error registrado ///
}DBContext;

int db_connect(DBContext *db);