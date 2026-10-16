
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Tarea infinita en el nucleo 1
void tareaInfinita(void *parametro) {

    volatile uint32_t contador = 0;

    while (true) {
        contador++;

        // No utilizar delay ni vTaskDelay
        // La tarea nunca libera voluntariamente el CPU
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("EJERCICIO 2 - WATCHDOG NUCLEO 1");
    Serial.println("Creando tarea infinita...");
    Serial.println("Nucleo: 1");
    Serial.println("Prioridad: 1");
    Serial.println("Esperando posible activacion del watchdog...");

    Serial.flush();

    xTaskCreatePinnedToCore(
        tareaInfinita,
        "TareaInfinita",
        4096,
        NULL,
        1,
        NULL,
        1
    );
}

void loop() {
    delay(1000);
}
