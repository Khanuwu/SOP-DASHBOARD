#include "opcua_client.h"
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <stdio.h>

#define OPCUA_ENDPOINT_URL "opc.tcp://10.99.184.11:4840"

static UA_Client *opcua_client = NULL;
static UA_Boolean opcua_last_valid = false;
static UA_Boolean opcua_last_is_double = false;
static double opcua_last_double = 0.0;
static int64_t opcua_last_int = 0;

static UA_Boolean opcua_extract_numeric(const UA_Variant *value,
                                        UA_Boolean *is_double,
                                        double *double_out,
                                        int64_t *int_out){
    if(!UA_Variant_isScalar(value) || value->data == NULL){
        return false;
    }

    if(value->type == &UA_TYPES[UA_TYPES_DOUBLE]){
        *is_double = true;
        *double_out = *(UA_Double*)value->data;
        return true;
    }
    if(value->type == &UA_TYPES[UA_TYPES_FLOAT]){
        *is_double = true;
        *double_out = *(UA_Float*)value->data;
        return true;
    }
    if(value->type == &UA_TYPES[UA_TYPES_INT16]){
        *is_double = false;
        *int_out = *(UA_Int16 *)value->data;
        return true;
    }
    if(value->type == &UA_TYPES[UA_TYPES_UINT16]){
        *is_double = false;
        *int_out = *(UA_UInt16 *)value->data;
        return true;
    }
    if(value->type == &UA_TYPES[UA_TYPES_INT32]){
        *is_double = false;
        *int_out = *(UA_Int32 *)value->data;
        return true;
    }
    if(value->type == &UA_TYPES[UA_TYPES_UINT32]){
        *is_double = false;
        *int_out = *(UA_UInt32 *)value->data;
        return true;
    }

    return false;
}

static void opcua_log_if_changed(UA_Boolean is_double,
                                 double double_value,
                                 int64_t int_value){
    if(is_double){
        if(!opcua_last_valid || !opcua_last_is_double || opcua_last_double != double_value){
            printf("OPC UA alarm code: %.2f\n", double_value);
            opcua_last_valid = true;
            opcua_last_is_double = true;
            opcua_last_double = double_value;
        }
        return;
    }

    if(!opcua_last_valid || opcua_last_is_double || opcua_last_int != int_value){
        printf("OPC UA alarm code: %lld\n", (long long)int_value);
        opcua_last_valid = true;
        opcua_last_is_double = false;
        opcua_last_int = int_value;
    }
}

UA_StatusCode opcua_client_init(void){
    if(opcua_client != NULL){
        return UA_STATUSCODE_GOOD;
    }

    opcua_client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(opcua_client));

    UA_StatusCode status = UA_Client_connect(opcua_client, OPCUA_ENDPOINT_URL);
    if(status != UA_STATUSCODE_GOOD){
        printf("OPC UA connection failed(0x%08x).\n", status);
        UA_Client_delete(opcua_client);
        opcua_client = NULL;
        return status;
    }

    return status;
}

UA_StatusCode opcua_client_read_alarm(void){
    if(opcua_client == NULL){
        UA_StatusCode status = opcua_client_init();
        if(status != UA_STATUSCODE_GOOD){
            return status;
        }
    }

    UA_Variant value;
    UA_Variant_init(&value);
    UA_NodeId alarm_node_id = OPCUA_TEMPERATURE_NODE_ID;
    UA_StatusCode status = UA_Client_readValueAttribute(opcua_client, alarm_node_id, &value);
    if(status == UA_STATUSCODE_GOOD){
        UA_Boolean is_double = false;
        double double_value = 0.0;
        int64_t int_value = 0;
        if(opcua_extract_numeric(&value, &is_double, &double_value, &int_value)){
            opcua_log_if_changed(is_double, double_value, int_value);
        }else{
            printf("OPC UA read value type is unsupported for logging.\n");
        }
    }else{
        printf("OPC UA read failed (0x%08x),\n", status);
    }

    UA_Variant_clear(&value);
    return status;
}

void opcua_client_shutdown(void){
    if(opcua_client == NULL){
        return;
    }

    UA_Client_disconnect(opcua_client);
    UA_Client_delete(opcua_client);
    opcua_client = NULL;
    opcua_last_valid = false;
}
