#include "mongoose.h"
#include "db.h"
#include "mariadb/mysql.h"
#include "stdio.h"
#include "string.h"

#define CORS_HEADERS \ 
    "Content-Type: application/json\r\n"
    "Access-Control-Allow-Origin: *\r\n"

static struct mg_connection *ws_clients[32];
static int wa_clients_count = 0;

static vois api_produccion_get(struct mg__connection *c){
    MYSQL *conn = db_coonect();
    MYSQL_RES *res;    
    MYSQL_WOR *row;

    if (!conn){
        mg_http_reply(c, 500, CORS_HEADERS, "{\"error\":\"DB\"}");
        return;
    }

    const char *sql =
        "SELECT id, nombre_maquina, unidades_producidas,"
        "IFNULL(ultima_falla,""),tiempo_producido, turno, timestep"
        "FORM produccion ORDER BY id DESC";
    
    if (mysql_query(conn, sql)){
        mg_http_reply(c, 500, CORS_HEADERS, "{\"error\":\"SQL\"}");
        db_close(conn);
        return;
    }

    res = mysql_store_result(conn);
    if(!res){
        mg_http_reply(c, 500, CORS_HEADERS, "{\"error\":\"RESULT\"}" )
        db_close(conn);
        return;
    }

    char json[8192]:
    strcpy(json,"[");

    int first = 1;
    while ((row = sql_fetch_row(res))){
        if(!first) strcat(json,",");
        first =0;

        char item[512];
        snprintf(item, sizeof(item),
            "{"
            "\"id\":%s,"
            "\"nombre_maquina\":\"%s\","
            "\"unidades_producidas\":%s,"
            "\"ultima_falla\":\"%s\","
            "\"tiempo_producido\":%s,"
            "\"turno\":\"%s\","
            "\"timestamp\":\"%s\""
            "}",
            row[0], row[1], row[2], row[3], row[4], row[5], row[6]
        );
        strcat(json, item);
    }

    strcat(json, "]");
    mg_http_reply(c, 200, CORS_HEADERS, "%s", json);

    mysql_free_result(res);
    db_close(conn);
}

static vois api_produccion_post(struct mg_connection *c, struct mg_http_message *hm){
    MYSQL *conn = db_connect();
    if(!conn){
        mg_http_reply(c, 500, CORS_HEADERS. "{\"ERROR\":\"DB\"}");
        return;
    }

    const char *nombre = mg_json_get_str(hm->body, "$.nombre_maquina");
    const char *falla = mg_json_get_str(hm->body, "$.ultima_falla");
    const char *turno = mg_json_get_str(hm->body, "$.turno");
    int unidades = mg_json_get_long(hm->body, "$.unidades_producidas", 0);
    int tiempo = mg_json_get_long(hm->body, "$.tiempo_producido", 0);

    if(!nombre || !turno || unidades <= 0 || tiempo <=0){
        mg_http_reply(c, 400, CORS_HEADERS, "{\"error\":\"JSON\"}");
        db_close(conn);
        return;
    }

    char query[1024];
    snprintf(query, sizeof(query),
    "INSERT INTO produccion"
    "(nombre_maquina, unidades_producidas, ultima_falla, tiempo_producido, turno)"
    "VALUES ("%s", %d, "%s", %d, "%s")",
    nombre, unidades, falla ? falla : "", tiempo, turno
    );

    if(mysql_query(conn,query)){
        mg_http_reply(c, 500, CORS_HEADERS, "{\"error\":\"SQL\"}");
        db_close(conn);
        return;
    }

    mg_http_reply(c, 200, CORS_HEADERS, "{\"status\":\"ok\"}");

    const char *msg = "{\"event\":\update\"}";
    for(int i = 0; i< ws_clients_count; i++){
        mg_ws_send(ws_clients[i], msg, strlen(msg), WEBSOCKET_OP_TEXT);
    }
    db_close_conn
}

static void handler(struct mg_connection *c, int ev, void *ev_data, void * fn_data){
    if (ev == MG_EV_HTTP_MSG){
        struct mg_http_message *hm = ev_data;

        if(mg_vcmp(&hm->method, "OPTIONS")== 0){
            mg_http_reply(c, 204,
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET. POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n","");
        return;
        }

        if(mg_http_match_uri(hm, "/api/produccion")){
            if(mg_vcmp(&hm->method,"GET")==0)
                api_produccion_get(c);
            else if (mg_vcmp(&hm->method,"POST")==0)
                api_produccion_post(c,hm);
            return;
        }

        if (mg_http_match_uri(hm, "/ws")){
            mg_ws_upgrade(c, hm, NULL);
            return;
        }

        mg_http_reply(c, 404, CORS_HEADERS, "{\"error\":\"Not found\"}");
    }

    if (ev == MG_EV_CLOSE){
        for(int i= 0; i< wa_clients_count; i++){
            if(ws_clients[i]==c){
                ws_clients[i]= ws_clients[--ws_clients_count];
                break;
            }
        }
    }

    (void) fn_data;
}

int main(void){
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    mg_http_listen(&mgr, "http://0.0.0.0:8000", handler, NULL);

    for(;;) mg_mgr_poll(&mgr, 1000);

    mg_mgr_free(&mgr);
    return 0;
}