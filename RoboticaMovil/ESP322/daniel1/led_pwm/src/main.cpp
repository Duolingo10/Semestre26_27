#include <Arduino.h>
#include <macros.h>

#define PWM_CANAL_A    0
#define PWM_FRECUENCIA 100
#define PWM_RESOLUCION 8

void setup() {
  pinMode(PIN_STBY, OUTPUT);  digitalWrite(PIN_STBY, LOW);
  pinMode(PIN_LED,  OUTPUT);
  ledcSetup(PWM_CANAL_A, PWM_FRECUENCIA, PWM_RESOLUCION);
  ledcAttachPin(PIN_LED, PWM_CANAL_A);
  ledcWrite(PWM_CANAL_A, 0); // fija ancho del pulso PWM en 0
}

void loop() {
  for(int i=0; i <= 255; i++) {
    ledcWrite(PWM_CANAL_A, i);
    delay(10);
  }
}