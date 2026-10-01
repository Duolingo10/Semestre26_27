#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Wire.begin(21, 22); // Pines I2C estándar del ESP32 (SDA=21, SCL=22)

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Dirección I2C común: 0x3C
    for (;;); // Detener ejecución si la pantalla no responde
  }

  display.clearDisplay();
  display.setTextSize(2);          // Tamaño de fuente (1 es pequeño, 2 mediano)
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(40, 24);       // Coordenadas (X, Y) para centrar el texto
  display.println("Hola");
  display.display();               // Envía los datos a la pantalla física
}

void loop() {
  // El loop permanece vacío para dejar el texto estático
}