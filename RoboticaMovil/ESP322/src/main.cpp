
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// 0 = Mutex (apartado a)
// 1 = Semaforo binario (apartado b)
#define USAR_BINARIO 1

SemaphoreHandle_t proteccionSerial;

// Funcion para imprimir el mensaje
void imprime(const char *nombre) {
    Serial.printf(
        "Mensaje de %s que corre en nucleo %d con prioridad %u\n",
        nombre,
        xPortGetCoreID(),
        (unsigned)uxTaskPriorityGet(NULL)
    );
}

// Protege la llamada a imprime()
void imprimirSeguro(const char *nombre) {
    if (xSemaphoreTake(proteccionSerial, portMAX_DELAY) == pdTRUE) {
        imprime(nombre);
        xSemaphoreGive(proteccionSerial);
    }
}

// Compatibilidad con distintas versiones de FreeRTOS
void esperaPeriodica(TickType_t *anterior, TickType_t periodo) {
#if defined(INCLUDE_xTaskDelayUntil) && (INCLUDE_xTaskDelayUntil == 1)
    xTaskDelayUntil(anterior, periodo);
#else
    vTaskDelayUntil(anterior, periodo);
#endif
}

// Tarea del nucleo 0
void tarea0(void *parametro) {
    TickType_t anterior = xTaskGetTickCount();

    for (;;) {
        imprimirSeguro("tarea0");
        esperaPeriodica(&anterior, pdMS_TO_TICKS(1000));
    }
}

// Tarea del nucleo 1
void tarea1(void *parametro) {
    TickType_t anterior = xTaskGetTickCount();

    for (;;) {
        imprimirSeguro("tarea1");
        esperaPeriodica(&anterior, pdMS_TO_TICKS(500));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

#if USAR_BINARIO
   Serial.println("Usando semaforo binario");
   proteccionSerial = xSemaphoreCreateBinary();
   if (proteccionSerial != NULL) {
       xSemaphoreGive(proteccionSerial);
   }
#else
   Serial.println("Usando mutex");
   proteccionSerial = xSemaphoreCreateMutex();
#endif

   if (proteccionSerial == NULL) {
        Serial.println("Error al crear la proteccion.");
        return;
    }

    xTaskCreatePinnedToCore(
        tarea0, "tarea0", 4096,
        NULL, 1, NULL, 0
    );

    xTaskCreatePinnedToCore(
        tarea1, "tarea1", 4096,
        NULL, 1, NULL, 1
    );
}

void loop() {
    delay(1000);
}
