#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
  char host[64];
  int port;
  char user[64];
  char password[64];
  char database[64];
  int connect_timeout;
  int read_timeout;
  int write_timeout;
} DBConfig;

int config_load(const char *path, DBConfig *cfg);



#endif  