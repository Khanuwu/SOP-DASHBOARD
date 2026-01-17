#ifndef OPCUA_CLIENT_H
#define OPCUA_CLIENT_H

#include<open62541/client.h>

#define OPCUA_TEMPERATURE_NODE_ID UA_NODEID_STRING(2, "Temperature")

UA_StatusCode opcua_client_read_temperature(void);

#endif