
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
    SCREEN_WIDTH, SCREEN_HEIGHT,
    &Wire, OLED_RESET
);

void setup() {

    // Inicializar la pantalla OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    // Limpiar la pantalla
    display.clearDisplay();

    // Dibujar las 7 lineas punteadas
    for(int i = 1; i < 8; i++) {

        for(int x = 0; x < 128; x += 4) {

            display.drawPixel(
                x,
                i * 8 - 1,
                WHITE
            );
        }
    }

    // Mostrar las lineas en la OLED
    display.display();
}

void loop() {

}
