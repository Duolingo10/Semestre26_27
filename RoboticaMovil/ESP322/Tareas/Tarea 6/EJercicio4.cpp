
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// 0 = Mutex
// 1 = Semaforo binario
#define USAR_BINARIO 0

SemaphoreHandle_t proteccionSerial;

struct Parametros {
    const char *nombre;
    uint32_t periodoMs;
};

Parametros datos0 = {"tarea0", 1000};
Parametros datos1 = {"tarea1", 500};

void imprime(const char *nombre) {
    Serial.printf(
        "Mensaje de %s que corre en nucleo %d con prioridad %u\n",
        nombre,
        xPortGetCoreID(),
        (unsigned)uxTaskPriorityGet(NULL)
    );
}

void esperaPeriodica(TickType_t *anterior, TickType_t periodo) {
#if defined(INCLUDE_xTaskDelayUntil) && (INCLUDE_xTaskDelayUntil == 1)
    xTaskDelayUntil(anterior, periodo);
#else
    vTaskDelayUntil(anterior, periodo);
#endif
}

// Misma funcion para las dos tareas
void tareaComun(void *parametro) {
    Parametros *datos = (Parametros *)parametro;

    TickType_t anterior = xTaskGetTickCount();
    TickType_t periodo = pdMS_TO_TICKS(datos->periodoMs);

    for (;;) {
        if (xSemaphoreTake(proteccionSerial, portMAX_DELAY) == pdTRUE) {
            imprime(datos->nombre);
            xSemaphoreGive(proteccionSerial);
        }

        esperaPeriodica(&anterior, periodo);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

#if USAR_BINARIO
    proteccionSerial = xSemaphoreCreateBinary();
    if (proteccionSerial != NULL) {
        xSemaphoreGive(proteccionSerial);
    }
#else
    proteccionSerial = xSemaphoreCreateMutex();
#endif

    if (proteccionSerial == NULL) {
        Serial.println("Error al crear la proteccion.");
        return;
    }

    // Misma funcion, nucleo 0, cada segundo
    xTaskCreatePinnedToCore(
        tareaComun,
        "tarea0",
        4096,
        &datos0,
        1,
        NULL,
        0
    );

    // Misma funcion, nucleo 1, cada 500 ms
    xTaskCreatePinnedToCore(
        tareaComun,
        "tarea1",
        4096,
        &datos1,
        1,
        NULL,
        1
    );
}

void loop() {
    delay(1000);
}
