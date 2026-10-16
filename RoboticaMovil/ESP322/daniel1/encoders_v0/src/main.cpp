#include <Arduino.h>
#include <macros.h>

volatile int g_pulsos_der = 0, g_pulsos_izq =0;

void IRAM_ATTR encoderA_izq() {
  if (digitalRead(PIN_ENCA_IZQ) == digitalRead(PIN_ENCB_IZQ)) 
    g_pulsos_izq--;
  else 
    g_pulsos_izq++;
}

void IRAM_ATTR encoderB_izq() {
  if (digitalRead(PIN_ENCA_IZQ) != digitalRead(PIN_ENCB_IZQ)) 
    g_pulsos_izq--;
  else 
    g_pulsos_izq++;
}

void IRAM_ATTR encoderA_der() {
  if (digitalRead(PIN_ENCA_DER) == digitalRead(PIN_ENCB_DER)) 
    g_pulsos_der--;
  else 
    g_pulsos_der++;
}

void IRAM_ATTR encoderB_der() {
  if (digitalRead(PIN_ENCA_DER) != digitalRead(PIN_ENCB_DER)) 
    g_pulsos_der--;
  else 
    g_pulsos_der++;
}

void setup() {
  Serial.begin(115200);
  pinMode (PIN_ENCA_IZQ, INPUT_PULLUP);
  pinMode (PIN_ENCB_IZQ, INPUT_PULLUP);
  pinMode (PIN_ENCA_DER, INPUT_PULLUP);
  pinMode (PIN_ENCB_DER, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCA_IZQ), encoderA_izq, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCB_IZQ), encoderB_izq, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCA_DER), encoderA_der, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCB_DER), encoderB_der, CHANGE);
}

void loop() {
  Serial.printf("izq = %4d  der = %4d \n", g_pulsos_izq, g_pulsos_der);
  delay(1000);
}