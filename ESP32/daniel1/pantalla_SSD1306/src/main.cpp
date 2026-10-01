#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET,400000UL,100000UL);

void setup() {
  Serial.begin(115200);
  Serial.print("display.begin ..."); Serial.flush();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Falla al inicializar SSD1306"));
    while(true) ; // espera por siempre
  }
  Serial.println("OK"); Serial.flush();
}

void loop() {
  display.clearDisplay();
  display.setTextSize(1); 
  display.setTextColor(WHITE);
  display.setCursor(0,0); // inicia en esquina superior izquierda
  display.println("Prueba de pantalla");
  display.println("Size = 1");
  display.println("8 renglones");
  display.println("21 caracteres");
  display.println("123456789012345678901");
  display.println("123456789012345678901");
  display.println("123456789012345678901");
  display.println("123456789012345678901");
  display.display();
  delay(5000);
  display.clearDisplay();
  display.setTextSize(2); 
  display.setCursor(0,0); // inicia en esquina superior izquierda
  display.println("Size = 2");
  display.println("4renglones");
  display.println("10 letras");
  display.println("1234567890");
  display.display();
  delay(5000);
}
