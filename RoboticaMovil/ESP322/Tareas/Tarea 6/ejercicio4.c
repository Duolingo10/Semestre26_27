
#include <Arduino.h>             // Funciones Arduino y comunicacion Serial.
#include "freertos/FreeRTOS.h"   // Definiciones del sistema FreeRTOS.
#include "freertos/task.h"       // Funciones para el manejo de tareas.
#include "freertos/semphr.h"     // Funciones de mutex y semaforos.

// Selecciona el mecanismo de sincronizacion.
// 0 utiliza mutex.
// 1 utiliza semaforo binario.
#define USAR_BINARIO 0

// Declara el manejador que protegera el puerto Serial.
SemaphoreHandle_t proteccionSerial;

// Define una estructura para almacenar informacion de cada tarea.
// struct permite agrupar varias variables en un mismo tipo de dato.
struct Parametros {

    // Puntero a una cadena que contiene el nombre de la tarea.
    const char *nombre;

    // Periodo de ejecucion expresado en milisegundos.
    // uint32_t es un entero sin signo de 32 bits.
    uint32_t periodoMs;
};

// Datos de la primera tarea.
// Su nombre sera tarea0 y su periodo sera de 1000 ms.
Parametros datos0 = {"tarea0", 1000};

// Datos de la segunda tarea.
// Su nombre sera tarea1 y su periodo sera de 500 ms.
Parametros datos1 = {"tarea1", 500};

// Funcion compartida para imprimir informacion de las tareas.
void imprime(const char *nombre) {

    // Imprime el nombre, nucleo y prioridad de la tarea.
    Serial.printf(
        // %s: cadena, %d: entero, %u: entero sin signo.
        "Mensaje de %s que corre en nucleo %d con prioridad %u\n",

        nombre,                       // Nombre de la tarea.
        xPortGetCoreID(),             // Nucleo actual: 0 o 1.
        (unsigned)uxTaskPriorityGet(NULL) // Prioridad de la tarea actual.
    );
}

// Funcion que permite esperar hasta la siguiente activacion.
// Recibe la referencia temporal y el periodo en ticks.
void esperaPeriodica(
    TickType_t *anterior,
    TickType_t periodo
) {

    // Verifica si esta habilitada la version xTaskDelayUntil().
#if defined(INCLUDE_xTaskDelayUntil) && \
    (INCLUDE_xTaskDelayUntil == 1)

    // Espera hasta el siguiente instante periodico.
    xTaskDelayUntil(anterior, periodo);

#else

    // Alternativa compatible con versiones anteriores.
    vTaskDelayUntil(anterior, periodo);

#endif // Termina la seleccion de la funcion.
}

// Funcion que utilizaran AMBAS tareas.
// Cada tarea recibira datos diferentes mediante parametro.
void tareaComun(void *parametro) {

    // Convierte el puntero generico void* en un puntero
    // a nuestra estructura Parametros.
    // Asi podemos acceder a nombre y periodoMs.
    Parametros *datos = (Parametros *)parametro;

    // Guarda el tick actual para la primera activacion.
    TickType_t anterior = xTaskGetTickCount();

    // Lee el periodo propio de la tarea.
    // Convierte los milisegundos a ticks de FreeRTOS.
    TickType_t periodo =
        pdMS_TO_TICKS(datos->periodoMs);

    // Ciclo infinito de ejecucion de la tarea.
    for (;;) {

        // Solicita el acceso exclusivo al puerto Serial.
        // Espera si otra tarea esta utilizando el recurso.
        if (xSemaphoreTake(
                proteccionSerial, // Mutex o semaforo.
                portMAX_DELAY     // Espera indefinida.
            ) == pdTRUE) {

            // INICIO DE LA SECCION CRITICA PROTEGIDA.

            // Obtiene el nombre desde la estructura.
            // El operador -> accede a un campo usando un puntero.
            imprime(datos->nombre);

            // Libera el recurso compartido.
            xSemaphoreGive(proteccionSerial);

            // FIN DE LA SECCION CRITICA PROTEGIDA.
        }

        // Suspende la tarea hasta su siguiente periodo.
        // Cada tarea utiliza su propio valor de periodo.
        esperaPeriodica(
            &anterior, // Referencia temporal individual.
            periodo    // Periodo obtenido de sus parametros.
        );
    }
}

// Configuracion inicial del ESP32.
void setup() {

    // Inicia la comunicacion con el monitor serial.
    Serial.begin(115200);

    // Espera un segundo.
    delay(1000);

    // Comprueba si se eligio semaforo binario.
#if USAR_BINARIO

    // Crea un semaforo binario.
    proteccionSerial = xSemaphoreCreateBinary();

    // Verifica si la creacion fue correcta.
    if (proteccionSerial != NULL) {

        // Entrega el permiso inicial del semaforo.
        xSemaphoreGive(proteccionSerial);
    }

#else

    // Si no se selecciono semaforo binario, crea un mutex.
    proteccionSerial = xSemaphoreCreateMutex();

#endif // Termina la seleccion.

    // Verifica si el mecanismo de proteccion existe.
    if (proteccionSerial == NULL) {

        // Muestra el error si no pudo crearse.
        Serial.println(
            "Error al crear la proteccion."
        );

        // Interrumpe la configuracion.
        return;
    }

    // Crea la primera tarea utilizando tareaComun().
    xTaskCreatePinnedToCore(
        tareaComun, // Funcion compartida para ambas tareas.
        "tarea0",   // Nombre interno de la primera tarea.
        4096,       // Tamaño de pila de esta tarea.
        &datos0,    // Direccion de los parametros de tarea0.
        1,          // Prioridad 1.
        NULL,       // No guardar identificador.
        0           // Asignar la tarea al nucleo 0.
    );

    // Crea la segunda tarea usando la MISMA funcion.
    xTaskCreatePinnedToCore(
        tareaComun, // Misma funcion que utiliza tarea0.
        "tarea1",   // Nombre interno de la segunda tarea.
        4096,       // Tamaño de pila de esta tarea.
        &datos1,    // Direccion de los parametros de tarea1.
        1,          // Prioridad 1.
        NULL,       // No guardar identificador.
        1           // Asignar la tarea al nucleo 1.
    );
}

// Funcion repetitiva de Arduino.
void loop() {

    // Suspende temporalmente el loop.
    // Las dos tareas continuan ejecutandose por FreeRTOS.
    delay(1000);
}
