
#include <Arduino.h>          // Funciones de Arduino: setup, loop, pinMode, digitalRead y delay.
#include <Adafruit_SSD1306.h> // Biblioteca para controlar la pantalla OLED SSD1306.
#include <Adafruit_GFX.h>     // Biblioteca para dibujar circulos, lineas, puntos y texto.
#include <math.h>            // Funciones matematicas como seno (sin) y coseno (cos).
#include <esp_timer.h>       // Temporizador del ESP32 con precision de microsegundos.

// ======================================================
// 1. CONFIGURACION DE LA PANTALLA OLED INTEGRADA
// ======================================================

#define SCREEN_WIDTH 128  // La pantalla tiene 128 pixeles de ancho.
#define SCREEN_HEIGHT 64  // La pantalla tiene 64 pixeles de alto.
#define OLED_RESET -1     // La pantalla no utiliza un pin de reinicio independiente.

// Creamos el objeto llamado display para controlar la pantalla.
// SCREEN_WIDTH y SCREEN_HEIGHT indican sus dimensiones.
// &Wire utiliza la interfaz I2C del ESP32.
// OLED_RESET indica la configuracion del reinicio.
// 400000UL establece la velocidad I2C durante la comunicacion.
// 100000UL es la velocidad I2C de restauracion.
Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET,
    400000UL,
    100000UL
);

// ======================================================
// 2. PIN DE CONTROL Y VARIABLES
// ======================================================

#define PIN_CONTROL 18 // GPIO 18: inicia o detiene el cronometro.

// Guarda el instante en que comienza o se reanuda el conteo.
uint64_t tiempoInicio = 0;

// Guarda el tiempo acumulado antes de la ultima pausa.
uint64_t tiempoAcumulado = 0;

// Indica si el cronometro esta funcionando.
// false = detenido; true = contando.
bool corriendo = false;

// Protege las variables compartidas entre las tareas.
// Evita que se lean y modifiquen simultaneamente.
portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;

// ======================================================
// 3. FUNCION PARA DIBUJAR EL RELOJ Y EL CONTADOR
// ======================================================

// Recibe el tiempo transcurrido en microsegundos.
// activo indica si el cronometro esta contando o detenido.
void dibujarReloj(uint64_t tiempo, bool activo) {

    // Borra la imagen anterior del buffer de la pantalla.
    display.clearDisplay();

    // Establece el color blanco para los dibujos y textos.
    display.setTextColor(WHITE);

    // Coordenadas del centro del reloj analogico.
    int cx = 31; // Posicion horizontal del centro.
    int cy = 32; // Posicion vertical del centro.

    // Radio del reloj en pixeles.
    int radio = 29;

    // Dibuja la circunferencia del reloj.
    display.drawCircle(cx, cy, radio, WHITE);

    // --------------------------------------------------
    // 4. DIBUJAR LAS 12 MARCAS DEL RELOJ
    // --------------------------------------------------

    // Repite el proceso 12 veces, una por cada marca.
    for (int i = 0; i < 12; i++) {

        // Cada marca esta separada 30 grados.
        // Se resta 90 para comenzar desde las 12.
        // Se convierte de grados a radianes.
        float angulo = (i * 30 - 90) * PI / 180.0;

        // Coordenadas del inicio de cada marca.
        // La distancia al centro es de 24 pixeles.
        int x1 = cx + 24 * cos(angulo);
        int y1 = cy + 24 * sin(angulo);

        // Coordenadas del final de cada marca.
        // La distancia al centro es de 27 pixeles.
        int x2 = cx + 27 * cos(angulo);
        int y2 = cy + 27 * sin(angulo);

        // Dibuja una linea pequena desde (x1,y1) hasta (x2,y2).
        display.drawLine(x1, y1, x2, y2, WHITE);
    }

    // --------------------------------------------------
    // 5. CALCULAR Y DIBUJAR LA MANECILLA
    // --------------------------------------------------

    // 60000000 microsegundos equivalen a 60 segundos.
    // El modulo (%) hace que la manecilla vuelva al inicio
    // despues de completar una vuelta de 60 segundos.
    // Se divide entre 1000000 para obtener segundos.
    float segundos = (tiempo % 60000000ULL) / 1000000.0;

    // La manecilla avanza 6 grados por segundo.
    // 360 grados / 60 segundos = 6 grados por segundo.
    // -90 coloca el inicio en la parte superior.
    // PI / 180 convierte el angulo a radianes.
    float angulo = (segundos * 6 - 90) * PI / 180.0;

    // Calcula el extremo horizontal de la manecilla.
    // La longitud de la manecilla es de 21 pixeles.
    int x = cx + 21 * cos(angulo);

    // Calcula el extremo vertical de la manecilla.
    int y = cy + 21 * sin(angulo);

    // Dibuja la manecilla desde el centro hasta (x,y).
    display.drawLine(cx, cy, x, y, WHITE);

    // Dibuja un pequeno circulo en el centro del reloj.
    display.fillCircle(cx, cy, 2, WHITE);

    // --------------------------------------------------
    // 6. CALCULAR LOS SEGUNDOS Y DECIMAS
    // --------------------------------------------------

    // Convierte microsegundos a segundos completos.
    // 1000000 microsegundos equivalen a 1 segundo.
    unsigned long seg = tiempo / 1000000ULL;

    // Obtiene la decima de segundo actual.
    // 100000 microsegundos equivalen a 0.1 segundos.
    // % 10 conserva solamente el digito de las decimas.
    unsigned long dec = (tiempo / 100000ULL) % 10;

    // --------------------------------------------------
    // 7. MOSTRAR EL TIEMPO EN LA PARTE DERECHA
    // --------------------------------------------------

    // Establece el tamano normal de la letra.
    display.setTextSize(1);

    // Coloca el cursor en la coordenada (70,12).
    display.setCursor(70, 12);

    // Escribe el titulo del contador.
    display.print("SEGUNDOS");

    // Posicion del contador digital.
    display.setCursor(67, 29);

    // Si hay menos de 1000 segundos, muestra letra grande.
    if (seg < 1000) {

        // Texto con el doble del tamano normal.
        display.setTextSize(2);

    } else {

        // Texto pequeno para numeros mas largos.
        display.setTextSize(1);
    }

    // Muestra los segundos completos.
    display.print(seg);

    // Coloca un punto decimal.
    display.print(".");

    // Muestra la cifra de las decimas.
    display.print(dec);

    // --------------------------------------------------
    // 8. MOSTRAR ESTADO DEL CRONOMETRO
    // --------------------------------------------------

    // Regresa al tamano normal de letra.
    display.setTextSize(1);

    // Coloca el cursor cerca de la parte inferior derecha.
    display.setCursor(68, 52);

    // Verifica si el cronometro esta activo.
    if (activo) {

        // Mensaje cuando esta contando.
        display.print("CONTANDO");

    } else {

        // Mensaje cuando esta detenido.
        display.print("DETENIDO");
    }

    // Envia la imagen completa del buffer a la pantalla.
    // Hasta este momento se visualizan los cambios.
    display.display();
}

// ======================================================
// 9. TAREA DE FREERTOS PARA ACTUALIZAR LA PANTALLA
// ======================================================

// Esta funcion se ejecuta como una tarea independiente.
// void *parametro permite recibir un argumento opcional.
void tareaPantalla(void *parametro) {

    // Obtiene el conteo actual de ticks de FreeRTOS.
    TickType_t tiempoAnterior = xTaskGetTickCount();

    // Convierte 100 milisegundos a ticks del sistema.
    // 100 milisegundos equivalen a 0.1 segundos.
    const TickType_t periodo = pdMS_TO_TICKS(100);

    // Ciclo infinito: la tarea sigue activa continuamente.
    while (true) {

        // Variable local para copiar el tiempo acumulado.
        uint64_t acumulado;

        // Variable local para copiar el instante inicial.
        uint64_t inicio;

        // Variable local para copiar el estado del cronometro.
        bool activo;

        // Entra a una seccion critica.
        // Evita cambios simultaneos de variables compartidas.
        portENTER_CRITICAL(&mutex);

        // Copia el tiempo acumulado compartido.
        acumulado = tiempoAcumulado;

        // Copia el ultimo instante de inicio.
        inicio = tiempoInicio;

        // Copia el estado del cronometro.
        activo = corriendo;

        // Sale de la seccion critica.
        portEXIT_CRITICAL(&mutex);

        // Obtiene el tiempo actual desde el arranque
        // del ESP32, expresado en microsegundos.
        uint64_t ahora = esp_timer_get_time();

        // Inicialmente utiliza el tiempo acumulado.
        uint64_t tiempo = acumulado;

        // Si el cronometro esta contando...
        if (activo) {

            // Calcula lo transcurrido desde el ultimo inicio
            // y lo suma al tiempo acumulado.
            tiempo += ahora - inicio;
        }

        // Dibuja el reloj y el contador digital.
        dibujarReloj(tiempo, activo);

        // Suspende la tarea hasta su siguiente activacion.
        // Mantiene un periodo programado de 100 ms.
        xTaskDelayUntil(&tiempoAnterior, periodo);
    }
}

// ======================================================
// 10. FUNCION SETUP: SE EJECUTA UNA SOLA VEZ
// ======================================================

void setup() {

    // Inicializa la comunicacion serial a 115200 baudios.
    Serial.begin(115200);

    // Configura GPIO 18 como entrada con pull-up interno.
    // Sin conectar a GND lee HIGH.
    // Conectado a GND lee LOW.
    pinMode(PIN_CONTROL, INPUT_PULLUP);

    // Informa que comenzara la inicializacion OLED.
    Serial.print("display.begin ...");

    // Espera a que se transmita el mensaje serial.
    Serial.flush();

    // Inicializa la OLED con direccion I2C 0x3C.
    // ! significa que la inicializacion fallo.
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

        // Muestra un mensaje si falla la pantalla.
        Serial.println(F("Falla al inicializar SSD1306"));

        // Detiene la ejecucion del programa.
        while (true);
    }

    // Indica que la pantalla inicio correctamente.
    Serial.println("OK");

    // Espera a terminar de transmitir el mensaje.
    Serial.flush();

    // Limpia el contenido del buffer de pantalla.
    display.clearDisplay();

    // Envia la pantalla vacia al modulo OLED.
    display.display();

    // --------------------------------------------------
    // 11. CREAR LA TAREA DE FREERTOS
    // --------------------------------------------------

    // Crea la tarea que actualiza la pantalla.
    xTaskCreate(
        tareaPantalla, // Funcion que ejecutara la tarea.
        "Pantalla",    // Nombre de la tarea.
        4096,          // Tamano de pila en bytes para ESP32.
        NULL,          // No recibe parametros adicionales.
        1,             // Prioridad de la tarea.
        NULL           // No se guarda un identificador.
    );
}

// ======================================================
// 12. LOOP: LECTURA DEL PIN DE CONTROL
// ======================================================

void loop() {

    // Lee el GPIO 18.
    // Si esta conectado a GND, digitalRead devuelve LOW.
    // La expresion sera true cuando este conectado.
    bool conectado = (digitalRead(PIN_CONTROL) == LOW);

    // Obtiene el tiempo actual en microsegundos.
    uint64_t ahora = esp_timer_get_time();

    // Protege las variables que comparte con tareaPantalla.
    portENTER_CRITICAL(&mutex);

    // Si el pin esta conectado y el cronometro no corria...
    if (conectado && !corriendo) {

        // Guarda el instante en el que se inicia o reanuda.
        tiempoInicio = ahora;

        // Cambia el estado a funcionando.
        corriendo = true;

    // Si esta desconectado y el cronometro estaba contando...
    } else if (!conectado && corriendo) {

        // Suma el tiempo de la ultima etapa al acumulado.
        tiempoAcumulado += ahora - tiempoInicio;

        // Cambia el estado a detenido.
        corriendo = false;
    }

    // Libera la seccion critica.
    portEXIT_CRITICAL(&mutex);

    // Espera 5 ms antes de volver a revisar el GPIO.
    // Tambien permite ejecutar otras tareas.
    delay(5);
}
