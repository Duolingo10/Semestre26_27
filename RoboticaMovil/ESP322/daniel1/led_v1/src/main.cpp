#include <Arduino.h>

#define PIN_LED 2         // Pin al que se conecta el led
#define TIEMPO_MS 500     // Tiempo de espera en milisegundos

void setup() {
  pinMode(PIN_LED, OUTPUT);
}
void loop() {
  digitalWrite(PIN_LED, HIGH);
  delay(TIEMPO_MS);
  digitalWrite(PIN_LED, LOW);
  delay(TIEMPO_MS);  
} 
