#include <Arduino.h>
#include "./macros.h"


void setup() {
  pinMode(PIN_LED,   OUTPUT);
  pinMode(PIN_BOTON, INPUT_PULLUP);
}

void loop() {
  int estado_boton;
  estado_boton = digitalRead(PIN_BOTON);
  if(estado_boton == HIGH)
    digitalWrite(PIN_LED, HIGH);
  else 
    digitalWrite(PIN_LED, LOW);
} 