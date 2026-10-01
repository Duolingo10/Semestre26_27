#include <Arduino.h>
#include <macros.h> // Define PIN_LED (2) y PIN_BOTON (15)

/* 
  Función lee_boton modificada:
  - Regresa -1 si el botón ya está presionado desde el inicio.
  - Regresa 0 si pasan 5 segundos de espera y NUNCA se oprime el botón.
  - Cuenta pulsaciones cortas (< 2 s) encendiendo el LED por 1 segundo en cada una.
  - Finaliza al detectar una pulsación larga (> 2 s), apaga el LED al soltar y regresa el conteo total.
*/
int lee_boton(int pin_boton, int pin_led, int max_segundos) {
    // Verificación si el botón ya está presionado al arrancar
    if (digitalRead(pin_boton) == LOW) {
        return -1; // Regresa -1 si el botón estaba oprimido desde el inicio
    }

    int contador = 0; // Variable para llevar la cuenta de las veces que se oprime el botón
    unsigned long tiempo_limite = max_segundos * 1000;; 

    while (true) {
        unsigned long t_inicio_espera = millis(); // Guarda el momento en que empieza a esperar una pulsación
        
        // Espera en este bucle mientras el botón NO esté presionado (HIGH)
        while (digitalRead(pin_boton) == HIGH) {
            // Si el botón nunca se presiona y se termina el tiempo límite de espera
            if (millis() - t_inicio_espera >= tiempo_limite) {
                if (contador == 0) {
                    return 0; // Regresa 0 porque NUNCA se oprimió el botón durante el tiempo de espera
                }
            }
            delay(10); 
        }

        delay(30);

        unsigned long t0 = millis();   // Guarda el tiempo inicial en que se presiona el botón
        bool es_presion_larga = false; // Bandera para saber si se superan los 2 segundos

        // Mientras el usuario mantenga oprimido el botón (LOW)
        while (digitalRead(pin_boton) == LOW) {
            // Si el botón se mantiene presionado por 2000 ms (2 segundos) o más
            if (millis() - t0 >= 2000) {
                es_presion_larga = true;     // Activa la marca de presión larga
                digitalWrite(pin_led, HIGH); // Enciende el LED mientras siga presionado
            }
            delay(10); 
        }

        delay(30); 

        // Evaluar qué acción realizar según la duración de la presión
        if (es_presion_larga) {
            // Caso 1: Presión larga de más de 2 segundos (última pulsación)
            digitalWrite(pin_led, LOW); // Apaga el LED al soltar el botón
            contador++;                 // Suma esta última ocasión presionado
            return contador;            
        } 
        else {
            // Caso 2: Pulsación corta (menos de 2 segundos)
            contador++;                  
            digitalWrite(pin_led, HIGH); // Enciende el LED por 1 segundo
            delay(1000);                 // Mantiene el LED encendido 1 segundo
            digitalWrite(pin_led, LOW);  // Apaga el LED
        }
    }
}

int opcion;

void setup() {
    Serial.begin(115200); // Inicializa la comunicación serial
    delay(2000);          // Tiempo de espera para abrir el Monitor Serie

    pinMode(PIN_BOTON, INPUT_PULLUP); 
    pinMode(PIN_LED, OUTPUT);         

    Serial.println("Inicio del programa boton_v3 MOD");
    Serial.println("Esperando 5 segundos por el boton...");

    // Llama a lee_boton pasando 5 segundos como tiempo máximo de espera
    opcion = lee_boton(PIN_BOTON, PIN_LED, 5);

    // Muestra el resultado final devuelto
    Serial.printf("la lectura del boton es: %d\n", opcion);
}

void loop() {

}