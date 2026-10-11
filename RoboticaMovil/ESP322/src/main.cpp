
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <math.h>
#include <esp_timer.h>

// Configuracion de la pantalla integrada
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
    SCREEN_WIDTH, SCREEN_HEIGHT,
    &Wire, OLED_RESET, 400000UL, 100000UL
);

// Pin para iniciar y detener
#define PIN_CONTROL 18

// Variables del cronometro
uint64_t tiempoInicio = 0;
uint64_t tiempoAcumulado = 0;
bool corriendo = false;

// Proteccion entre tareas
portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;

// Funcion para dibujar el reloj
void dibujarReloj(uint64_t tiempo, bool activo) {

    display.clearDisplay();
    display.setTextColor(WHITE);

    // Centro y radio del reloj
    int cx = 31;
    int cy = 32;
    int radio = 29;

    // Dibujar circulo
    display.drawCircle(cx, cy, radio, WHITE);

    // Dibujar las 12 marcas
    for (int i = 0; i < 12; i++) {

        float angulo = (i * 30 - 90) * PI / 180.0;

        int x1 = cx + 24 * cos(angulo);
        int y1 = cy + 24 * sin(angulo);

        int x2 = cx + 27 * cos(angulo);
        int y2 = cy + 27 * sin(angulo);

        display.drawLine(x1, y1, x2, y2, WHITE);
    }

    // Calcular segundos para la manecilla
    float segundos = (tiempo % 60000000ULL) / 1000000.0;

    // 6 grados por segundo
    float angulo = (segundos * 6 - 90) * PI / 180.0;

    int x = cx + 21 * cos(angulo);
    int y = cy + 21 * sin(angulo);

    // Dibujar manecilla
    display.drawLine(cx, cy, x, y, WHITE);
    display.fillCircle(cx, cy, 2, WHITE);

    // Tiempo digital con decimas
    unsigned long seg = tiempo / 1000000ULL;
    unsigned long dec = (tiempo / 100000ULL) % 10;

    display.setTextSize(1);
    display.setCursor(70, 12);
    display.print("SEGUNDOS");

    // Mostrar segundos y decimas
    display.setCursor(67, 29);

    if (seg < 1000) {
        display.setTextSize(2);
    } else {
        display.setTextSize(1);
    }

    display.print(seg);
    display.print(".");
    display.print(dec);

    // Estado del cronometro
    display.setTextSize(1);
    display.setCursor(68, 52);

    if (activo) {
        display.print("CONTANDO");
    } else {
        display.print("DETENIDO");
    }

    display.display();
}

// Tarea FreeRTOS: actualiza cada 100 ms
void tareaPantalla(void *parametro) {

    TickType_t tiempoAnterior = xTaskGetTickCount();
    const TickType_t periodo = pdMS_TO_TICKS(100);

    while (true) {

        uint64_t acumulado;
        uint64_t inicio;
        bool activo;

        // Obtener estado del cronometro
        portENTER_CRITICAL(&mutex);

        acumulado = tiempoAcumulado;
        inicio = tiempoInicio;
        activo = corriendo;

        portEXIT_CRITICAL(&mutex);

        // Calcular tiempo real transcurrido
        uint64_t ahora = esp_timer_get_time();
        uint64_t tiempo = acumulado;

        if (activo) {
            tiempo += ahora - inicio;
        }

        // Actualizar graficos
        dibujarReloj(tiempo, activo);

        // Despertar cada decima de segundo
        xTaskDelayUntil(&tiempoAnterior, periodo);
    }
}

void setup() {

    Serial.begin(115200);

    // GPIO con resistencia pull-up interna
    pinMode(PIN_CONTROL, INPUT_PULLUP);

    Serial.print("display.begin ...");
    Serial.flush();

    // Inicializar pantalla integrada
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("Falla al inicializar SSD1306"));
        while (true);
    }

    Serial.println("OK");
    Serial.flush();

    display.clearDisplay();
    display.display();

    // Crear tarea de actualizacion
    xTaskCreate(
        tareaPantalla,
        "Pantalla",
        4096,
        NULL,
        1,
        NULL
    );
}

void loop() {

    // LOW = conectado a tierra
    bool conectado = (digitalRead(PIN_CONTROL) == LOW);

    uint64_t ahora = esp_timer_get_time();

    portENTER_CRITICAL(&mutex);

    if (conectado && !corriendo) {

        // Iniciar o continuar
        tiempoInicio = ahora;
        corriendo = true;

    } else if (!conectado && corriendo) {

        // Pausar cronometro
        tiempoAcumulado += ahora - tiempoInicio;
        corriendo = false;
    }

    portEXIT_CRITICAL(&mutex);

    delay(5);
}
