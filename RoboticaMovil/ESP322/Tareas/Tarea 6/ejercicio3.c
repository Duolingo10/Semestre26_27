
#include <Arduino.h>             // Funciones de Arduino y comunicacion Serial.
#include "freertos/FreeRTOS.h"   // Definiciones principales de FreeRTOS.
#include "freertos/task.h"       // Funciones para crear y controlar tareas.
#include "freertos/semphr.h"     // Funciones de mutex y semaforos.

// Selecciona el mecanismo de proteccion.
// 0 significa utilizar un mutex.
// 1 significa utilizar un semaforo binario.
#define USAR_BINARIO 0

// Declara un manejador para el recurso de sincronizacion.
// SemaphoreHandle_t identifica el mutex o semaforo creado.
// Se comparte entre las dos tareas.
SemaphoreHandle_t proteccionSerial;

// Funcion encargada de imprimir el mensaje de cada tarea.
// const char *nombre recibe el nombre como una cadena de texto.
// const indica que la funcion no modificara ese texto.
void imprime(const char *nombre) {

    // printf permite combinar texto con valores de variables.
    Serial.printf(
        // %s representa una cadena de texto.
        // %d representa un numero entero.
        // %u representa un entero sin signo.
        // \n produce un salto de linea.
        "Mensaje de %s que corre en nucleo %d con prioridad %u\n",

        nombre,                     // Nombre recibido como parametro.
        xPortGetCoreID(),           // Obtiene el nucleo que ejecuta esta funcion.
        (unsigned)uxTaskPriorityGet(NULL) // Obtiene la prioridad de la tarea actual.
    );
}

// Funcion que protege el acceso compartido al puerto Serial.
// Recibe el nombre de la tarea que desea imprimir.
void imprimirSeguro(const char *nombre) {

    // Intenta obtener el mutex o semaforo.
    // xSemaphoreTake devuelve pdTRUE si lo obtiene.
    // portMAX_DELAY indica esperar indefinidamente si esta ocupado.
    if (xSemaphoreTake(
            proteccionSerial, // Recurso de sincronizacion.
            portMAX_DELAY     // Tiempo maximo de espera.
        ) == pdTRUE) {

        // INICIO DE LA SECCION CRITICA PROTEGIDA.
        // Solo una tarea que posee el recurso puede entrar.
        imprime(nombre);

        // Libera el recurso para que otra tarea pueda utilizarlo.
        xSemaphoreGive(proteccionSerial);

        // FIN DE LA SECCION CRITICA PROTEGIDA.
    }
}

// Funcion auxiliar para mantener una ejecucion periodica.
// TickType_t es el tipo de dato usado por FreeRTOS
// para contar los ticks del sistema.
// anterior guarda la referencia del ultimo inicio periodico.
// periodo representa cuantos ticks deben pasar entre activaciones.
void esperaPeriodica(
    TickType_t *anterior,
    TickType_t periodo
) {

    // Comprueba si la version instalada tiene habilitada
    // la funcion xTaskDelayUntil().
#if defined(INCLUDE_xTaskDelayUntil) && \
    (INCLUDE_xTaskDelayUntil == 1)

    // Suspende la tarea hasta su siguiente instante periodico.
    // Tambien actualiza el instante de referencia.
    xTaskDelayUntil(anterior, periodo);

#else

    // En versiones anteriores se utiliza vTaskDelayUntil().
    // Cumple el mismo proposito de espera periodica.
    vTaskDelayUntil(anterior, periodo);

#endif // Fin de la seleccion de version.
}

// Primera tarea: se ejecuta en el nucleo 0.
void tarea0(void *parametro) {

    // Obtiene el tick actual como referencia inicial.
    TickType_t anterior = xTaskGetTickCount();

    // Ciclo infinito que mantiene viva la tarea.
    for (;;) {

        // Llama a la funcion de impresion protegida.
        imprimirSeguro("tarea0");

        // Espera hasta la siguiente activacion periodica.
        esperaPeriodica(
            &anterior,            // Direccion de la referencia temporal.
            pdMS_TO_TICKS(1000)   // Convierte 1000 ms a ticks.
        );

        // El ciclo se repite aproximadamente cada segundo.
    }
}

// Segunda tarea: se ejecuta en el nucleo 1.
void tarea1(void *parametro) {

    // Guarda el tick actual como referencia temporal.
    TickType_t anterior = xTaskGetTickCount();

    // Mantiene la tarea ejecutandose indefinidamente.
    for (;;) {

        // Llama a la misma funcion de impresion protegida.
        imprimirSeguro("tarea1");

        // Programa la siguiente activacion cada 500 ms.
        esperaPeriodica(
            &anterior,           // Direccion de la referencia temporal.
            pdMS_TO_TICKS(500)   // Convierte 500 ms a ticks.
        );

        // Por cada segundo se esperan dos ejecuciones.
    }
}

// Se ejecuta una sola vez al iniciar el ESP32.
void setup() {

    // Inicializa la comunicacion Serial.
    Serial.begin(115200);

    // Espera un segundo para estabilizar el inicio.
    delay(1000);

    // Comprueba la constante USAR_BINARIO.
    // Si vale 1, compila el codigo del semaforo binario.
#if USAR_BINARIO

    // Crea un semaforo binario.
    // Inicialmente no tiene un permiso disponible.
    proteccionSerial = xSemaphoreCreateBinary();

    // Comprueba si el semaforo se creo correctamente.
    if (proteccionSerial != NULL) {

        // Entrega un permiso inicial al semaforo.
        // Esto permite que la primera tarea pueda utilizarlo.
        xSemaphoreGive(proteccionSerial);
    }

#else

    // Si USAR_BINARIO vale 0, crea un mutex.
    // Un mutex nuevo comienza disponible.
    proteccionSerial = xSemaphoreCreateMutex();

#endif // Termina la seleccion del mecanismo.

    // Comprueba si fallo la creacion del recurso.
    // NULL significa que no se obtuvo un manejador valido.
    if (proteccionSerial == NULL) {

        // Muestra un mensaje de error.
        Serial.println(
            "Error al crear la proteccion."
        );

        // Termina setup sin crear las tareas.
        return;
    }

    // Crea la primera tarea.
    xTaskCreatePinnedToCore(
        tarea0,    // Funcion que ejecutara la tarea.
        "tarea0",  // Nombre interno de la tarea.
        4096,      // Tamaño de pila reservado.
        NULL,      // No recibe parametros.
        1,         // Prioridad 1.
        NULL,      // No se guarda el identificador.
        0          // Ejecutar en el nucleo 0.
    );

    // Crea la segunda tarea.
    xTaskCreatePinnedToCore(
        tarea1,    // Funcion que ejecutara la tarea.
        "tarea1",  // Nombre interno de la tarea.
        4096,      // Tamaño de pila reservado.
        NULL,      // No recibe parametros.
        1,         // Prioridad 1.
        NULL,      // No se guarda el identificador.
        1          // Ejecutar en el nucleo 1.
    );
}

// Funcion principal del programa Arduino.
void loop() {

    // Deja transcurrir un segundo.
    // Las otras tareas se ejecutan independientemente.
    delay(1000);
}
