
#include <Arduino.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Cambiar este valor en cada experimento
const uint32_t DURACION_MS = 3500;

void tareaBloqueo(void *parametro) {
    Serial.printf(
        "Inicio de bloqueo del nucleo 0: %lu ms\n",
        (unsigned long)DURACION_MS
    );
    Serial.flush();

    int64_t inicio = esp_timer_get_time();

    // Bucle ocupado: no permite ejecutar IDLE0
    while ((esp_timer_get_time() - inicio) <
           ((int64_t)DURACION_MS * 1000)) {
        // No usar delay() ni vTaskDelay()
    }

    Serial.println("Bloqueo terminado sin reinicio.");
    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

#ifdef CONFIG_ESP_TASK_WDT_TIMEOUT_S
    Serial.printf(
        "Timeout configurado: %d segundos\n",
        CONFIG_ESP_TASK_WDT_TIMEOUT_S
    );
#endif

    xTaskCreatePinnedToCore(
        tareaBloqueo,
        "Bloqueo0",
        4096,
        NULL,
        1,
        NULL,
        0
    );
}

void loop() {
    delay(1000);
}
