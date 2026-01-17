# OPC UA client (open62541)

Este módulo usa **open62541** para conectarse a un endpoint OPC UA y leer un NodeId fijo.

## Parámetros de prueba

- **Endpoint:** `opc.tcp://localhost:4840`
- **NodeId:** `ns=2;s=Temperature`

La función `opcua_client_read_temperature()` lee ese NodeId y escribe el valor en consola para validar la conexión.