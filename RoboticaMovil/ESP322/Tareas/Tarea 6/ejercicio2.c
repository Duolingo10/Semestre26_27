
#include <Arduino.h>             // Incluye las funciones basicas de Arduino.
#include "freertos/FreeRTOS.h"   // Incluye el sistema operativo FreeRTOS.
#include "freertos/task.h"       // Incluye funciones para administrar tareas.

// Define una funcion que ejecutara una tarea infinita.
// El parametro es obligatorio en la firma de una tarea FreeRTOS,
// aunque en este ejercicio no necesitamos utilizarlo.
void tareaInfinita(void *parametro) {

    // Crea un contador entero sin signo de 32 bits.
    // Inicia en cero.
    // volatile indica al compilador que no debe eliminar
    // ni optimizar indebidamente sus accesos.
    volatile uint32_t contador = 0;

    // Ciclo infinito que nunca termina por si mismo.
    while (true) {

        // Incrementa el contador una unidad.
        contador++;

        // No utilizamos delay() ni vTaskDelay().
        // Por ello, la tarea no libera voluntariamente
        // el nucleo 1 para que se ejecute IDLE1.
    }
}

// Se ejecuta una sola vez al encender o reiniciar el ESP32.
void setup() {

    // Configura la comunicacion con el monitor serial.
    Serial.begin(115200);

    // Espera un segundo antes de comenzar la prueba.
    delay(1000);

    // Informa que se comenzara a crear una tarea.
    Serial.println("Creando tarea...");

    // Muestra el nucleo asignado a la nueva tarea.
    Serial.println("Nucleo: 1");

    // Muestra la prioridad asignada.
    Serial.println("Prioridad: 1");

    // Informa que se observara si aparece un fallo.
    Serial.println(
        "Esperando posible activacion del watchdog..."
    );

    // Espera a terminar de enviar los mensajes por Serial.
    Serial.flush();

    // Crea una tarea infinita en el nucleo 1.
    xTaskCreatePinnedToCore(
        tareaInfinita,   // Funcion que ejecutara la tarea.
        "TareaInfinita", // Nombre interno de la tarea.
        4096,            // Tamaño de pila de la tarea.
        NULL,            // No recibe parametros.
        1,               // Prioridad 1, mayor que IDLE1.
        NULL,            // No guardamos su identificador.
        1                // Asignacion al nucleo 1.
    );
}

// Funcion repetitiva principal de Arduino.
void loop() {

    // Espera un segundo en cada repeticion del loop.
    // Esta espera no detiene la tareaInfinita.
    delay(1000);
}
