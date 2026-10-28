
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <math.h>
#include <string.h>

// =====================================
// CONFIGURACION DE LA PANTALLA
// =====================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET,
    400000UL,
    100000UL
);

// =====================================
// PINES DEL ESP32
// =====================================

// Entrada del control del Arduino
#define PIN_ENTRADA 34

// Salida de velocidad hacia Arduino
#define PIN_SALIDA 25

// =====================================
// CONFIGURACION DEL MOTOR
// =====================================

// true = generar señal de prueba
// false = leer señal del Arduino
const bool modoDemo = false;

// Modelo de la practica 4
const float a = 0.146773f;
const float b = 0.853227f;

// Periodo de muestreo: 10 ms
const unsigned long h = 10000;

// Numero de muestras por pista
#define NUM_MUESTRAS 118

// Voltaje virtual maximo
const float VMAX = 5.0f;

// =====================================
// VARIABLES DEL MOTOR
// =====================================

float entrada = 0.0f;
float salida = 0.0f;

int lecturaADC = 0;
int milivoltios = 0;

bool perturbacion = false;

// Arreglos de las dos señales
float muestrasU[NUM_MUESTRAS] = {};
float muestrasY[NUM_MUESTRAS] = {};

int indice = 0;
int cantidad = 0;

// Proteccion entre tareas
portMUX_TYPE mutexDatos =
    portMUX_INITIALIZER_UNLOCKED;

// =====================================
// CONVERTIR VOLTAJE A PIXELES
// =====================================

int convertirY(
    float voltaje,
    int arriba,
    int abajo
) {

    voltaje = constrain(
        voltaje, 0.0f, VMAX
    );

    return abajo - roundf(
        voltaje * (abajo - arriba) / VMAX
    );
}

// =====================================
// DIBUJAR PISTAS
// =====================================

void dibujarPistas() {

    // Separador punteado
    for(int x = 0; x < 128; x += 4) {
        display.drawPixel(x, 32, WHITE);
    }
}

// =====================================
// DIBUJAR UNA SENAL
// =====================================

void dibujarSenal(
    float datos[],
    int inicio,
    int total,
    int arriba,
    int abajo
) {

    int yActual;
    int yAnterior;

    for(int i = 0; i < total; i++) {

        int posicion =
            (inicio + i) % NUM_MUESTRAS;

        int x = i + 10;

        yActual = convertirY(
            datos[posicion],
            arriba,
            abajo
        );

        display.drawPixel(
            x, yActual, WHITE
        );

        if(i > 0) {

            int anterior =
                (inicio + i - 1)
                % NUM_MUESTRAS;

            yAnterior = convertirY(
                datos[anterior],
                arriba,
                abajo
            );

            display.drawLine(
                x - 1, yAnterior,
                x, yActual,
                WHITE
            );
        }
    }
}

// =====================================
// GRAFICAR EN LA OLED
// =====================================

void graficar() {

    float copiaU[NUM_MUESTRAS];
    float copiaY[NUM_MUESTRAS];

    float uActual;
    float yActual;

    int adcActual;
    int mvActual;

    int total;
    int inicio;

    // Copiar los datos protegidos
    portENTER_CRITICAL(&mutexDatos);

    memcpy(
        copiaU,
        muestrasU,
        sizeof(muestrasU)
    );

    memcpy(
        copiaY,
        muestrasY,
        sizeof(muestrasY)
    );

    uActual = entrada;
    yActual = salida;

    adcActual = lecturaADC;
    mvActual = milivoltios;

    total = cantidad;
    inicio = indice;

    portEXIT_CRITICAL(&mutexDatos);

    if(total < NUM_MUESTRAS) {
        inicio = 0;
    }

    // Limpiar pantalla
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);

    dibujarPistas();

    // =================================
    // PRIMERA PISTA: CONTROL ARDUINO
    // =================================

    display.setCursor(0, 0);

    display.print("U ARD:");
    display.print(uActual, 2);

    dibujarSenal(
        copiaU,
        inicio,
        total,
        11,
        30
    );

    // =================================
    // SEGUNDA PISTA: MOTOR ESP32
    // =================================

    display.setCursor(0, 35);

    display.print("Y ESP:");
    display.print(yActual, 2);

    dibujarSenal(
        copiaY,
        inicio,
        total,
        45,
        54
    );

    // =================================
    // DIAGNOSTICO DEL ADC
    // =================================

    display.setCursor(0, 56);

    display.print("ADC:");
    display.print(adcActual);

    display.print(" mV:");
    display.print(mvActual);

    // Actualizar pantalla
    display.display();
}

// =====================================
// TAREA DEL OSCILOSCOPIO
// =====================================

void tareaOsciloscopio(void *parameter) {

    TickType_t tiempoAnterior =
        xTaskGetTickCount();

    while(true) {

        // Dibujar las dos señales
        graficar();

        // Actualizar cada 100 ms
        vTaskDelayUntil(
            &tiempoAnterior,
            pdMS_TO_TICKS(100)
        );
    }
}

// =====================================
// CONFIGURACION INICIAL
// =====================================

void setup() {

    Serial.begin(115200);

    // Entrada analogica
    pinMode(PIN_ENTRADA, INPUT);

    // Resolucion ADC de 12 bits
    analogReadResolution(12);

    // Atenuacion de entrada
    analogSetPinAttenuation(
        PIN_ENTRADA,
        ADC_11db
    );

    // Inicializar salida DAC en cero
    dacWrite(PIN_SALIDA, 0);

    // Inicializar comunicacion I2C
    Wire.begin(21, 22);

    // Iniciar pantalla OLED
    if(!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
    )) {

        Serial.println(
            "Error al inicializar OLED"
        );

        while(true) {
            delay(1000);
        }
    }

    // Mensaje de inicio
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0, 0);

    display.println("CONTROL DIGITAL");
    display.println("MOTOR VIRTUAL");
    display.println("ESP32 HW-454C");
    display.println("DIAGNOSTICO ADC");

    display.display();

    delay(1000);

    // Crear tarea OLED en nucleo 0
    xTaskCreatePinnedToCore(
        tareaOsciloscopio,
        "tareaOsciloscopio",
        4096,
        NULL,
        1,
        NULL,
        0
    );

    Serial.println();
    Serial.println("====================");
    Serial.println("MOTOR VIRTUAL ESP32");
    Serial.println("ADC CORREGIDO");
    Serial.println("GPIO34: ENTRADA");
    Serial.println("GPIO25: SALIDA DAC");
    Serial.println("MUESTREO: 10 ms");
    Serial.println("====================");
}

// =====================================
// PROGRAMA PRINCIPAL
// =====================================

void loop() {

    static unsigned long tiempoAnterior =
        micros();

    static unsigned long tiempoSerial = 0;

    unsigned long ahora = micros();

    // =================================
    // COMANDOS DE PERTURBACION
    // =================================

    while(Serial.available()) {

        char comando = Serial.read();

        // Activar perturbacion
        if(comando == 'p') {

            portENTER_CRITICAL(&mutexDatos);

            perturbacion = true;

            portEXIT_CRITICAL(&mutexDatos);
        }

        // Desactivar perturbacion
        if(comando == 'n') {

            portENTER_CRITICAL(&mutexDatos);

            perturbacion = false;

            portEXIT_CRITICAL(&mutexDatos);
        }
    }

    // =================================
    // PERIODO DE MUESTREO
    // =================================

    if((unsigned long)(
        ahora - tiempoAnterior
    ) < h) {

        delay(1);
        return;
    }

    tiempoAnterior += h;

    float u = 0.0f;

    int adcActual = 0;
    int mvActual = 0;

    // =================================
    // LEER SENAL DE ENTRADA
    // =================================

    if(modoDemo) {

        // Tren de pulsos de 1 Hz
        // Amplitud de 2.5 V
        u = (
            millis() % 1000UL < 500UL
        ) ? 2.5f : 0.0f;

    } else {

        // Leer valor crudo del ADC
        adcActual = analogRead(
            PIN_ENTRADA
        );

        // Leer voltaje calibrado
        mvActual = analogReadMilliVolts(
            PIN_ENTRADA
        );

        float voltaje;

        // =================================
        // CORRECCION DEL CERO DEL ADC
        // =================================

        // Evitar los 142 mV falsos
        // cuando el ADC crudo vale cero
        if(adcActual <= 5) {

            voltaje = 0.0f;

        } else {

            voltaje =
                mvActual / 1000.0f;
        }

        // =================================
        // RECUPERAR VOLTAJE DEL ARDUINO
        // =================================

        // Divisor resistivo 3:1
        // Arduino 5 V -> ESP32 1.67 V
        u = voltaje * 3.0f;

        // Limitar voltaje virtual
        u = constrain(
            u,
            0.0f,
            VMAX
        );
    }

    // =================================
    // MOSTRAR DIAGNOSTICO SERIAL
    // =================================

    if(millis() - tiempoSerial >= 500) {

        tiempoSerial = millis();

        Serial.print("ADC: ");
        Serial.print(adcActual);

        Serial.print(" | mV: ");
        Serial.print(mvActual);

        Serial.print(" | U: ");
        Serial.print(u, 3);

        Serial.print(" | Y: ");
        Serial.println(salida, 3);
    }

    // =================================
    // LEER PERTURBACION
    // =================================

    portENTER_CRITICAL(&mutexDatos);

    bool carga = perturbacion;

    portEXIT_CRITICAL(&mutexDatos);

    float entradaMotor = u;

    // Aplicar perturbacion virtual
    if(carga) {

        entradaMotor -= 0.8f;

        if(entradaMotor < 0.0f) {
            entradaMotor = 0.0f;
        }
    }

    // =================================
    // MODELO DISCRETO DEL MOTOR
    // =================================

    // y[k+1] = b*y[k] + a*u[k]
    float nuevaSalida =
        b * salida + a * entradaMotor;

    nuevaSalida = constrain(
        nuevaSalida,
        0.0f,
        VMAX
    );

    // =================================
    // GENERAR SALIDA DAC
    // =================================

    // Escala virtual: 0 a 5 V
    // Escala fisica: 0 a 3.3 V
    int valorDAC = roundf(
        nuevaSalida * 255.0f / 5.0f
    );

    valorDAC = constrain(
        valorDAC,
        0,
        255
    );

    // Enviar velocidad al Arduino
    dacWrite(
        PIN_SALIDA,
        valorDAC
    );

    // =================================
    // GUARDAR DATOS PARA LA OLED
    // =====================================

    portENTER_CRITICAL(&mutexDatos);

    entrada = u;
    salida = nuevaSalida;

    lecturaADC = adcActual;
    milivoltios = mvActual;

    muestrasU[indice] = entrada;
    muestrasY[indice] = salida;

    indice =
        (indice + 1) % NUM_MUESTRAS;

    if(cantidad < NUM_MUESTRAS) {
        cantidad++;
    }

    portEXIT_CRITICAL(&mutexDatos);
}
