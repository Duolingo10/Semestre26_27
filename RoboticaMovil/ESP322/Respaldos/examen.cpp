#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>


//Configuración de la pantalla
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Pines utilizados
#define PIN_SALIDA 2
#define PIN_ENTRADA 4

//muestras
#define NUM_MUESTRAS 1000

//I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 400000UL, 100000UL);

//arreglo para guardar las muestras
int muestras[NUM_MUESTRAS];
//Variables del generador
int frecuencia = 30;
int estado = LOW;
int etapa = 0;

unsigned long mitadPeriodo = 16667;
unsigned long tiempoAnterior = 0;
unsigned long tiempoPulso = 0;

//lineas punteadas
void dibujarPistas() {
    for(int i = 1; i < 8; i++) {
        int y = i * 8 - 1;
        for(int x = 0; x < 128; x += 4) {
            display.drawPixel(x, y, WHITE);
        }
    }
}
//graficar las 1000 muestras
void graficar() {
    int pista, x, yAlto, yBajo;
    int yActual, yAnterior;

    display.clearDisplay();

    // Dibujar los separadores
    dibujarPistas();
    for(int i = 0; i < NUM_MUESTRAS; i++){
        pista = i / 128;//carril y columna
        x = i % 128;
    

    //posiciones para HIGH y LOW
    yAlto = pista * 8 + 1;
    yBajo = pista * 8 + 5;

    if(muestras[i] == HIGH){
        yActual = yAlto;
    } else {
        yActual = yBajo;
    }

    //punto actual
    display.drawPixel(x, yActual, WHITE);
    if(x > 0){
        if(muestras[i - 1] == HIGH){
            yAnterior = yAlto;
        } else {
            yAnterior = yBajo;
        }

        //linea horizontal
        display.drawLine(x - 1, yAnterior, x, yAnterior, WHITE);

        //linea vertical
        display.drawLine(x, yAnterior, x, yActual, WHITE);
    }
    display.display();
}
}
//tarea del osciloscopio
void tareaOsciloscopio(void *parameter) {
    TickType_t ticks0, inc_ms = pdMS_TO_TICKS(1);
    while(true){
        ticks0 = xTaskGetTickCount();
        for(int i = 0; i < NUM_MUESTRAS; i++){
            muestras[i] = digitalRead(PIN_ENTRADA);
            xTaskDelayUntil(&ticks0, inc_ms);
        }
        graficar();
        delay(2000);
    }
}
void setup() {
    Serial.begin(115200);
    pinMode(PIN_SALIDA, OUTPUT);
    pinMode(PIN_ENTRADA, INPUT_PULLUP);
    digitalWrite(PIN_SALIDA, LOW);

    Serial.print("display.begin ...");//
    Serial.flush();
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
        Serial.println(F("Falla al inicializar SSD1306"));
        while(true);
    }
    Serial.println("OK");
    Serial.flush();
    display.clearDisplay();
    //lienas punteadas
    dibujarPistas();
    display.display();

    //tiempos
    tiempoAnterior = millis();
    tiempoPulso = micros();

    //tarea en nucleo 0
    xTaskCreatePinnedToCore(tareaOsciloscopio, "tareaOsciloscopio", 4096, NULL, 1, NULL, 0);

    Serial.printf("Frecuencia: %d Hz\n", frecuencia);
}

void loop(){
    unsigned long tiempoActual = millis();
    //Frecuencia cada 1/3 ms
    if(tiempoActual - tiempoAnterior >= 333){
        tiempoAnterior += 333;
        etapa++;
        if(etapa >= 3){
            etapa = 0;
    }
    //Frecuencia de 30 Hz
    if(etapa == 0){
        frecuencia = 30;
        mitadPeriodo = 16667;
    }
    //Frecuencia de 20 Hz
    if(etapa == 1){
        frecuencia = 20;
        mitadPeriodo = 25000;
    }
    //Frecuencia de 10 Hz
    if(etapa == 2){
        frecuencia = 10;
        mitadPeriodo = 50000;
    }
    Serial.printf("Frecuencia: %d Hz\n", frecuencia);
    }

    //Generar señal cuadrada
    unsigned long tiempoActualMicros = micros();
    if(tiempoActualMicros - tiempoPulso >= mitadPeriodo){
        tiempoPulso += mitadPeriodo;
        estado = !estado;
        digitalWrite(PIN_SALIDA, estado);
}
}