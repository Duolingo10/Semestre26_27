
#include <Arduino.h>             // Incluye las funciones de Arduino y Serial.
#include "freertos/FreeRTOS.h"   // Incluye la configuracion del sistema FreeRTOS.
#include "freertos/task.h"       // Incluye funciones para consultar tareas.

// Funcion ejecutada una sola vez al iniciar el ESP32.
void setup() {

    // Configura la comunicacion Serial a 115200 baudios.
    Serial.begin(115200);

    // Espera un segundo para estabilizar el inicio.
    delay(1000);

    // Imprime el nombre de la funcion que vamos a investigar.
    Serial.println("Verificando vTaskList...");

    // Muestra el valor de configUSE_TRACE_FACILITY.
    // Esta opcion habilita funciones de seguimiento de tareas.
    // 0 significa deshabilitada.
    // 1 significa habilitada.
    Serial.printf(
        "TRACE_FACILITY: %d\n",
        configUSE_TRACE_FACILITY
    );

    // Muestra el valor de configUSE_STATS_FORMATTING_FUNCTIONS.
    // Esta opcion permite generar estadisticas
    // y listados de tareas en formato de texto.
    // 0 significa deshabilitada.
    // 1 significa habilitada.
    Serial.printf(
        "STATS_FORMATTING: %d\n",
        configUSE_STATS_FORMATTING_FUNCTIONS
    );

    // El preprocesador comprueba las dos configuraciones.
    // El operador && significa "Y" logico.
    // Ambas opciones deben valer 1 para utilizar vTaskList().
#if (configUSE_TRACE_FACILITY == 1) && \
    (configUSE_STATS_FORMATTING_FUNCTIONS == 1)

    // Crea un arreglo de 4096 caracteres.
    // Sirve para almacenar el texto generado por vTaskList().
    // {0} inicializa el arreglo con caracteres nulos.
    char lista[4096] = {0};

    // Indica que se mostrara el listado de tareas.
    Serial.println("Listado de tareas:");

    // Obtiene informacion de las tareas existentes.
    // Guarda el resultado como texto dentro del arreglo lista.
    // Esta funcion requiere estar incluida en FreeRTOS
    // durante la compilacion del framework.
    vTaskList(lista);

    // Imprime la informacion obtenida por vTaskList().
    Serial.println(lista);

#else

    // Si alguna configuracion vale 0,
    // se compila esta parte del programa.
    // En nuestro ESP32 ambas configuraciones dieron 0.
    Serial.println(
        "vTaskList no esta habilitada en esta compilacion."
    );

    // Consulta cuantas tareas existen actualmente.
    // uxTaskGetNumberOfTasks() devuelve el numero total
    // de tareas que FreeRTOS tiene registradas.
    Serial.printf(
        "Numero de tareas: %u\n",
        (unsigned)uxTaskGetNumberOfTasks()
    );

    // En nuestro experimento se detectaron 6 tareas.
    // Estas pueden incluir tareas internas de FreeRTOS,
    // tareas del framework Arduino y tareas del usuario.

#endif // Termina la comprobacion de las configuraciones.
}

// Funcion que Arduino ejecuta repetidamente.
void loop() {

    // Espera un segundo en cada repeticion.
    // No volvemos a imprimir porque la comprobacion
    // se realiza una sola vez dentro de setup().
    delay(1000);
}
