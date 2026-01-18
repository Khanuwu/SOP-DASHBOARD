# OPC UA client (open62541)

Este módulo usa **open62541** para conectarse a un endpoint OPC UA y leer un NodeId fijo.

## Parámetros de prueba

- **Endpoint:** `opc.tcp://localhost:4840`
- **NodeId:** `ns=2;s=Temperature`

La función `opcua_client_read_temperature()` lee ese NodeId y escribe el valor en consola para validar la conexión.

## Nota de compilación

Los errores de *undefined reference* a símbolos `UA_*` indican que falta enlazar la librería
de **open62541**. Asegurate de incluirla en el link, por ejemplo agregando `-lopen62541`
o el path correspondiente si compilás de forma manual.