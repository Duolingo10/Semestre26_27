
#include <Arduino.h>             // Funciones de Arduino: Serial y delay().
#include "esp_timer.h"           // Permite medir el tiempo en microsegundos.
#include "freertos/FreeRTOS.h"   // Incluye las definiciones de FreeRTOS.
#include "freertos/task.h"       // Permite crear, administrar y eliminar tareas.

// Define cuantos milisegundos permanecera ocupado el nucleo 0.
// Cambiando este valor realizamos las diferentes pruebas.
const uint32_t DURACION_MS = 4999;

// Funcion que sera ejecutada como una tarea de FreeRTOS.
// void: no devuelve ningun valor.
// void *parametro: permite recibir datos al crear la tarea.
// En este ejercicio no necesitamos utilizar ese parametro.
void tareaBloqueo(void *parametro) {

    // Muestra cuanto tiempo intentaremos bloquear el nucleo.
    Serial.printf(
        "Inicio de bloqueo del nucleo 0: %lu ms\n",
        (unsigned long)DURACION_MS
    );

    // Espera a que terminen de enviarse los datos por Serial.
    // Asi evitamos comenzar el bloqueo antes de enviar el mensaje.
    Serial.flush();

    // Guarda el tiempo exacto en que comienza el bloqueo.
    // esp_timer_get_time() devuelve microsegundos.
    // int64_t permite almacenar un numero entero de 64 bits.
    int64_t inicio = esp_timer_get_time();

    // Comienza el bloqueo del nucleo 0.
    // Obtiene el tiempo actual y le resta el tiempo inicial.
    // Compara el tiempo transcurrido con la duracion solicitada.
    while ((esp_timer_get_time() - inicio) <
           ((int64_t)DURACION_MS * 1000)) {

        // El ciclo permanece ocupado sin ejecutar delay().
        // Se multiplica por 1000 porque 1 ms = 1000 us.
        // La tarea no libera voluntariamente el procesador.
        // Esto impide que IDLE0, de prioridad 0, se ejecute.
    }

    // Este mensaje aparece si el bloqueo termina
    // antes de que el watchdog aborte el programa.
    Serial.println("Bloqueo terminado sin reinicio.");

    // Elimina la tarea actual porque termino su trabajo.
    // NULL indica que se elimina la tarea que llama a la funcion.
    vTaskDelete(NULL);
}

// Funcion que Arduino ejecuta una sola vez al iniciar.
void setup() {

    // Inicia la comunicacion serial a 115200 baudios.
    Serial.begin(115200);

    // Espera 1000 milisegundos para estabilizar el inicio.
    delay(1000);

    // Comprueba si existe la constante de configuracion
    // del tiempo de espera del watchdog.
#ifdef CONFIG_ESP_TASK_WDT_TIMEOUT_S

    // Muestra el timeout configurado en segundos.
    Serial.printf(
        "Timeout configurado: %d segundos\n",
        CONFIG_ESP_TASK_WDT_TIMEOUT_S
    );

#endif // Termina la comprobacion del preprocesador.

    // Crea una tarea y la asigna a un nucleo especifico.
    xTaskCreatePinnedToCore(
        tareaBloqueo,  // Funcion que ejecutara la tarea.
        "Bloqueo0",    // Nombre de la tarea.
        4096,          // Tamaño de pila reservado para la tarea.
        NULL,          // No enviamos parametros a la funcion.
        1,             // Prioridad 1: mayor que la de IDLE0.
        NULL,          // No necesitamos guardar su identificador.
        0              // Ejecutar exclusivamente en el nucleo 0.
    );
}

// Funcion principal repetitiva del framework Arduino.
void loop() {

    // Suspende temporalmente el loop durante 1000 ms.
    // No interviene en el bloqueo realizado por tareaBloqueo.
    delay(1000);
}
