
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// ============================================
// CONFIGURACION DE LA PANTALLA OLED
// ============================================

//Para no levantar sospechas hagamos lo siguiente:
//Dame cada bloque de codigo por separado, al final me das todo el codigo junto.
/*  1. Imprimimos en el oled las lineas punteadas que separan las 8 pistas de la pantalla
    2. Agregamos la linea donde se graficara la señal de entrada (de preferencia que se vea asi _-_-)
    3. Hacemos el generador de señales cuadradas en el pin 2, que cambie de frecuencia cada 333 ms (30, 20 y 10 Hz)
    4. Hacemos la captura de 1000 muestras en el pin 4
    5. Graficamos las 1000 muestras en la pantalla OLED

Debes entregarme cada bloque de codigo por separado, no todo junto.
El codigo tiene que estar bien estructurado y comentado, y debe compilar sin errores.
Tambien es importante que logres un 100% de similitud con los codigos del libro.
    */

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Pines utilizados
#define PIN_SALIDA 2
#define PIN_ENTRADA 4

// Numero total de muestras
#define NUM_MUESTRAS 1000

// Objeto de la pantalla OLED
Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET,
    400000UL,
    100000UL
);

// Arreglo para almacenar las 1000 muestras
int muestras[NUM_MUESTRAS];

// ============================================
// VARIABLES DEL GENERADOR
// ============================================

// Frecuencia inicial: 30 Hz
int frecuencia = 30;

// Medio periodo en microsegundos
// 30 Hz = 16667 us
// 20 Hz = 25000 us
// 10 Hz = 50000 us
unsigned long mitadPeriodo = 16667;

// Estado de la salida digital
int estado = LOW;

// Etapa actual del generador
// 0 = 30 Hz
// 1 = 20 Hz
// 2 = 10 Hz
int etapa = 0;

// Variables para controlar los tiempos
unsigned long tiempoAnterior = 0;
unsigned long tiempoPulso = 0;


// ============================================
// FUNCION PARA GRAFICAR LAS MUESTRAS
// ============================================

void graficar() {

    int pista;
    int columna;

    int yAlto;
    int yBajo;

    int yActual;
    int yAnterior;

    // Limpiar la memoria de la pantalla
    display.clearDisplay();

    // Recorrer las 1000 muestras
    for(int i = 0; i < NUM_MUESTRAS; i++) {

        // Determinar en que pista va la muestra
        pista = i / 128;

        // Determinar la posicion horizontal
        columna = i % 128;

        // Definir los niveles alto y bajo
        // Cada pista ocupa 8 pixeles de altura
        yAlto = pista * 8 + 1;
        yBajo = pista * 8 + 5;

        // Revisar el valor de la muestra
        if(muestras[i] == HIGH) {
            yActual = yAlto;
        }
        else {
            yActual = yBajo;
        }

        // Dibujar la muestra actual
        display.drawPixel(
            columna,
            yActual,
            WHITE
        );

        // Unir la muestra actual con la anterior
        if(columna > 0) {

            // Obtener la altura de la muestra anterior
            if(muestras[i - 1] == HIGH) {
                yAnterior = yAlto;
            }
            else {
                yAnterior = yBajo;
            }

            // Dibujar la linea entre las muestras
            display.drawLine(
                columna - 1,
                yAnterior,
                columna,
                yActual,
                WHITE
            );
        }
    }

    // Dibujar las 7 lineas punteadas
    // que separan las 8 pistas
    for(int i = 1; i < 8; i++) {

        for(int x = 0; x < 128; x += 4) {

            display.drawPixel(
                x,
                i * 8 - 1,
                WHITE
            );
        }
    }

    // Mostrar la grafica completa
    display.display();
}


// ============================================
// TAREA DEL OSCILOSCOPIO
// ============================================

void tareaOsciloscopio(void *parameter) {

    TickType_t tiempoAnteriorMuestra;

    // Intervalo entre muestras: 1 ms
    TickType_t intervalo = pdMS_TO_TICKS(1);

    while(true) {

        // Obtener el tiempo inicial
        tiempoAnteriorMuestra = xTaskGetTickCount();

        // Capturar 1000 muestras
        for(int i = 0; i < NUM_MUESTRAS; i++) {

            // Leer el GPIO 4
            muestras[i] = digitalRead(PIN_ENTRADA);

            // Esperar hasta el siguiente periodo
            xTaskDelayUntil(
                &tiempoAnteriorMuestra,
                intervalo
            );
        }

        // Graficar todas las muestras
        graficar();

        // Mantener la grafica 2 segundos
        delay(2000);
    }
}


// ============================================
// CONFIGURACION INICIAL
// ============================================

void setup() {

    // Iniciar comunicacion serial
    Serial.begin(115200);

    // GPIO 2 como salida
    pinMode(PIN_SALIDA, OUTPUT);

    // GPIO 4 como entrada con pull-up
    pinMode(PIN_ENTRADA, INPUT_PULLUP);

    // Iniciar salida en nivel bajo
    digitalWrite(PIN_SALIDA, LOW);

    // Inicializar pantalla OLED
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

        Serial.println("Error al iniciar OLED");

        while(true);
    }

    // Limpiar pantalla al iniciar
    display.clearDisplay();
    display.display();

    // Crear tarea del osciloscopio
    // Ejecutar en el nucleo 0
    xTaskCreatePinnedToCore(
        tareaOsciloscopio,
        "Osciloscopio",
        10000,
        NULL,
        1,
        NULL,
        0
    );

    // Inicializar tiempos del generador
    tiempoAnterior = millis();
    tiempoPulso = micros();

    Serial.println("Sistema iniciado");
    Serial.println("Generador: 30, 20, 10 Hz");
}


// ============================================
// GENERADOR DE SENALES CUADRADAS
// ============================================

void loop() {

    unsigned long tiempoActual;
    unsigned long tiempoActualPulso;

    // Obtener tiempo actual en milisegundos
    tiempoActual = millis();

    // Cambiar de frecuencia cada 333 ms
    if(tiempoActual - tiempoAnterior >= 333) {

        // Actualizar tiempo de referencia
        tiempoAnterior += 333;

        // Avanzar a la siguiente etapa
        etapa++;

        // Reiniciar la secuencia
        if(etapa == 3) {
            etapa = 0;
        }

        // Primera etapa: 30 Hz
        if(etapa == 0) {

            frecuencia = 30;
            mitadPeriodo = 16667;
        }

        // Segunda etapa: 20 Hz
        if(etapa == 1) {

            frecuencia = 20;
            mitadPeriodo = 25000;
        }

        // Tercera etapa: 10 Hz
        if(etapa == 2) {

            frecuencia = 10;
            mitadPeriodo = 50000;
        }

        // Mostrar frecuencia en monitor serie
        Serial.print("Frecuencia: ");
        Serial.print(frecuencia);
        Serial.println(" Hz");
    }

    // Obtener tiempo en microsegundos
    tiempoActualPulso = micros();

    // Comprobar si transcurrio medio periodo
    if(tiempoActualPulso - tiempoPulso >= mitadPeriodo) {

        // Actualizar referencia temporal
        tiempoPulso = tiempoActualPulso;

        // Cambiar estado de la salida
        if(estado == LOW) {
            estado = HIGH;
        }
        else {
            estado = LOW;
        }

        // Escribir el nuevo estado en GPIO 2
        digitalWrite(PIN_SALIDA, estado);
    }
}
