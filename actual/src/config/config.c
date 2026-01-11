/*//////////////Inicializa config
                    Inicializa DB
                    Inicializa OPC UA
                    Arranca loop principal
                    Maneja shutdown limpio//////////////////*/

#include <stdio.h> //f open,fgets,fclose para abrir y leer archivo de configuracion
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "config.h"

static char *ltrim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    return s;
}

static void rtrim(char *s) {
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
}

int config_load(const char *path, DBConfig *cfg)
{
    FILE *f = fopen(path, "r");
    if (!f || !cfg) return -1;

    memset(cfg, 0, sizeof(DBConfig));

    char line[256];

    while (fgets(line, sizeof(line), f)) {

        char *p = ltrim(line);
        rtrim(p);

        if (*p == '\0' || *p == '#' || *p == ';' || *p == '[')
            continue;

        char *eq = strchr(p, '=');
        if (!eq) continue;

        *eq = '\0';

        char *key = ltrim(p);
        char *value = ltrim(eq + 1);
        rtrim(key);
        rtrim(value);

        if (strcmp(key, "host") == 0)
            strncpy(cfg->host, value, sizeof(cfg->host) - 1);
        else if (strcmp(key, "port") == 0)
            cfg->port = atoi(value);
        else if (strcmp(key, "user") == 0)
            strncpy(cfg->user, value, sizeof(cfg->user) - 1);
        else if (strcmp(key, "password") == 0)
            strncpy(cfg->password, value, sizeof(cfg->password) - 1);
        else if (strcmp(key, "database") == 0)
            strncpy(cfg->database, value, sizeof(cfg->database) - 1);
        else if(strcmp(key, "ssl_ca") == 0)
            strncpy(cfg->ssl_ca, value, sizeof(cfg->ssl_ca) - 1);
        else if(strcmp(key,"ssl_cert") == 0)
            strncpy(cfg->ssl_cert, value, sizeof(cfg->ssl_cert) -1);
        else if(strcmp(key, "ssl_key") == 0)
            strncpy(cfg->ssl_key, value, sizeof(cfg->ssl_key) -1);
        else if (strcmp(key, "connect_timeout_sec") == 0)
            cfg->connect_timeout = atoi(value);
        else if (strcmp(key, "read_timeout_sec") == 0)
            cfg->read_timeout = atoi(value);
        else if (strcmp(key, "write_timeout_sec") == 0)
            cfg->write_timeout = atoi(value);
    }

    fclose(f);

    if (cfg->password[0] == '\0'){
        const char *env_password = getenv("DB_PASSWORD");
        if(env_password && env_password[0] != '\0'){
            strncpy(cfg->password, env_password, sizeof(cfg->password) - 1);
        }
    }

    /* Validación mínima */
    if (cfg->host[0] == '\0') return -1;
    if (cfg->port <= 0) return -1;
    if (cfg->user[0] == '\0') return -1;
    if (cfg->database[0] == '\0') return -1;

    printf("CFG host='%s'\n", cfg->host);
    printf("CFG port=%d\n", cfg->port);

    return 0;
}
