#include <Arduino.h>
#include <macros.h>

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println(analogRead(PIN_BOTON));
  delay(1000); 
}