#include <Arduino.h>
#include <macros.h>

void setup() {
  pinMode(PIN_LED,   OUTPUT);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  digitalWrite(PIN_LED,LOW);
  while(digitalRead(PIN_BOTON) == HIGH)
    ;
  digitalWrite(PIN_LED,HIGH);
}
void loop() {} 