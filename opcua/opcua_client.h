#ifndef OPCUA_CLIENT_H
#define OPCUA_CLIENT_H

#include<open62541/client.h>

#define OPCUA_TEMPERATURE_NODE_ID \
    UA_NODEID_STRING(2, "Application.Variables_Dfb.Message.Defaut")

UA_StatusCode opcua_client_init(void);
UA_StatusCode opcua_client_read_alarm(void);
void opcua_client_shutdown(void);

#endif
