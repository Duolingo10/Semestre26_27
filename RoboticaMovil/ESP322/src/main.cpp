
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Verificando vTaskList...");

    Serial.printf(
        "TRACE_FACILITY: %d\n",
        configUSE_TRACE_FACILITY
    );

    Serial.printf(
        "STATS_FORMATTING: %d\n",
        configUSE_STATS_FORMATTING_FUNCTIONS
    );

#if (configUSE_TRACE_FACILITY == 1) && \
    (configUSE_STATS_FORMATTING_FUNCTIONS == 1)

    char lista[4096] = {0};

    Serial.println("Listado de tareas:");
    vTaskList(lista);
    Serial.println(lista);

#else

    Serial.println(
        "vTaskList no esta habilitada en esta compilacion."
    );

    Serial.printf(
        "Numero de tareas: %u\n",
        (unsigned)uxTaskGetNumberOfTasks()
    );

#endif
}

void loop() {
    delay(1000);
}
