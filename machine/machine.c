#include "machine.h"
#include "data_model.h"

// Construye un snapshot de máquina a partir de entradas crudas.
MachineSnapshot machine_snapshot_from_raw(bool running,
                                          uint32_t total_units,
                                          uint32_t good_units,
                                          uint32_t reject_units,
                                          uint16_t alarm_code,
                                          const char *alarm_description,
                                          bool alarm_active) {
    // Mapea el estado, contadores y alarma con los helpers del modelo.
    MachineSnapshot snapshot = {
        .status = machine_status_from_running(running),
        .counter = unit_counter_from_raw(total_units, good_units, reject_units),
        .alarm = alarm_cause_from_raw(alarm_code, alarm_description, alarm_active),
    };

    return snapshot;
}

   