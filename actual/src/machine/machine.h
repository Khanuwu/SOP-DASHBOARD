#ifndef MACHINE_H
#define MACHINE_H

#include <stdbool.h>
#include <stdint.h>

#include "data_model.h"

// Snapshot agregado de la máquina para un instante de muestreo.
typedef struct {
    MachineStatus status; // Ejemplo: { .state = MACHINE_STATE_RUNNING }
    UnitCounter counter;  // Ejemplo: { .total_units = 12500, .good_units = 12450, .reject_units = 50 }
    AlarmCause alarm;     // Ejemplo: { .alarm_code = 203, .description = "Sensor timeout", .active = true }
} MachineSnapshot;

// Mapea entradas crudas a una única estructura de snapshot.
MachineSnapshot machine_snapshot_from_raw(bool running,
                                          uint32_t total_units,
                                          uint32_t good_units,
                                          uint32_t reject_units,
                                          uint16_t alarm_code,
                                          const char *alarm_description,
                                          bool alarm_active);

#endif // MACHINE_H