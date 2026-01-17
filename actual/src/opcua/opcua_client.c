#include "opcua_client.h"
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <stdio.h>

#define OPCUA_ENDPOINT_URL "opc.tcp://localhost:4840"

static void opcua_log_read_value(const UA_Variant *value){
    /* Validar que la respuesta sea escalar y tenga datos. */
    if(!UA_Variant_isScalar(value)|| value->data == NULL){
        printf("OPC UA read value is not a scalar.\n");
        return;
    }
    /* Soportar tipos numéricos comunes para logging. */
    if(value->type == &UA_TYPES[UA_TYPES_DOUBLE]){
        double temp = *(UA_Double*)value->data;
        printf("OPC UA temperature value: %.2f\n", temp);
        return;
    }
    if(value->type == &UA_TYPES[UA_TYPES_FLOAT]){
        float temp = *(UA_Float*)value->data;
        printf("OPC UA temperature value:%.2f\n", temp);
        return;
    }
    if(value->type == &UA_TYPES[UA_TYPES_INT32]){
        int32_t temp =*(UA_Int32 *)value->data;
        printf("OPA UA temperature value: %d\n", temp);
        return;
    }
    if(value->type == &UA_TYPES[UA_TYPES_INT32]){
        uint32_t temp = *(UA_UInt32 *)value->data;
        printf("OPC UA temperature value : %u\n", temp);
        return;
    }

    printf("OPC UA read value type is unsupported for logging.\n");

}

UA_StatusCode opcua_client_read_temperature(void){
    /* Crear cliente y cargar configuración por defecto. */
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));

    /* Conectar al endpoint configurado. */
    UA_StatusCode status = UA_Client_connect(client, OPCUA_ENDPOINT_URL);
    if(status != UA_STATUSCODE_GOOD){
        printf("OPC UA connection failed(0x%08x).\n", status);
        UA_Client_delete(client);
        return status;
    }

    /* Leer el valor del NodeId fijo. */
    UA_Variant value;
    UA_Variant_init(&value);
    UA_NodeId temperature_node_id = OPCUA_TEMPERATURE_NODE_ID;
    status = UA_Client_readValueAttribute(client, temperature_node_id, &value);
    if(status == UA_STATUSCODE_GOOD){
        opcua_log_read_value(&value);
    }else{
        printf("OPC UA read failed (0x%08x),\n", status);
    }

    /* Limpiar recursos y cerrar conexión. */
    UA_Variant_clear(&value);
    UA_Client_disconnect(client);
    UA_Client_delete(client);
    return status;
}
