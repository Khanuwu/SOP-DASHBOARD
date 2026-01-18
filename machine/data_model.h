#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include <stdbool.h>
#include <stdint.h>

// Representa si la máquina está en marcha o detenida.
typedef enum {
    MACHINE_STATE_STOPPED = 0, // Ejemplo: 0 (detenida)
    MACHINE_STATE_RUNNING = 1  // Ejemplo: 1 (en marcha)
} MachineState;

// Registra el estado actual de la máquina.
typedef struct {
    MachineState state; // Estado actual. Ejemplo: MACHINE_STATE_RUNNING
} MachineStatus;

// Mapea un flag booleano de marcha a un MachineStatus.
static inline MachineStatus machine_status_from_running(bool running) {
    MachineStatus status = { .state = running ? MACHINE_STATE_RUNNING : MACHINE_STATE_STOPPED };
    return status;
}

// Registra los contadores de unidades producidas por la máquina.
typedef struct {
    uint32_t total_units;  // Total de unidades producidas. Ejemplo: 12500
    uint32_t good_units;   // Unidades buenas producidas. Ejemplo: 12450
    uint32_t reject_units; // Unidades rechazadas. Ejemplo: 50
} UnitCounter;

// Mapea valores crudos de contadores a un UnitCounter.
static inline UnitCounter unit_counter_from_raw(uint32_t total,
                                                 uint32_t good,
                                                 uint32_t reject) {
    UnitCounter counter = { .total_units = total, .good_units = good, .reject_units = reject };
    return counter;
}

// Describe una alarma o causa de paro en la máquina.
typedef struct {
    uint16_t alarm_code;      // Código numérico de alarma. Ejemplo: 203
    const char *description;  // Descripción de la alarma. Ejemplo: "Sensor timeout"
    bool active;              // Indica si la alarma está activa. Ejemplo: true
} AlarmCause;

// Mapea información cruda de alarmas a un AlarmCause.
static inline AlarmCause alarm_cause_from_raw(uint16_t code,
                                              const char *description,
                                              bool active) {
    AlarmCause cause = { .alarm_code = code, .description = description, .active = active };
    return cause;
}

#endif // DATA_MODEL_H