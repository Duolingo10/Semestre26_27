
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
    SCREEN_WIDTH, SCREEN_HEIGHT,
    &Wire, OLED_RESET, 400000UL, 100000UL
);

void setup() {

    Serial.begin(115200);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Falla al inicializar SSD1306");
        while(true);
    }
}

void loop() {

    display.clearDisplay();

    // Dibujar una señal de prueba en las 8 pistas
    for(int pista = 0; pista < 8; pista++) {

        int yAlto = pista * 8 + 1;
        int yBajo = pista * 8 + 5;

        // Cada 16 pixeles cambia el nivel
        for(int x = 0; x < 128; x++) {

            int nivel = (x / 16) % 2;
            int yActual;

            if(nivel == 0) {
                yActual = yBajo;
            }
            else {
                yActual = yAlto;
            }

            // Dibujar la linea horizontal
            display.drawPixel(x, yActual, WHITE);

            // Dibujar una linea vertical al cambiar de nivel
            if(x > 0 && x % 16 == 0) {
                display.drawLine(
                    x, yAlto,
                    x, yBajo,
                    WHITE
                );
            }
        }
    }

    // Dibujar separadores punteados
    for(int i = 1; i < 8; i++) {
        for(int x = 0; x < 128; x += 4) {
            display.drawPixel(x, i * 8 - 1, WHITE);
        }
    }

    display.display();

    delay(2000);
}
