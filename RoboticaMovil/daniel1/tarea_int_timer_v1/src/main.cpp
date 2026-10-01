#include <Arduino.h>
#include <ESP32TimerInterrupt.h>  // Instalar la biblioteca ESP32TimerInterrupt en Code
#include <macros.h>

#define TIEMPO_MICROSEG    (1000*1000)

ESP32Timer ITimer0(0);

bool IRAM_ATTR funcion_timer(void * timerNo) { 
  static bool estado = false;
  digitalWrite(PIN_LED, estado);
  estado = !estado;
  return true;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_STBY,OUTPUT); digitalWrite(PIN_STBY,LOW);
  pinMode(PIN_LED, OUTPUT);
  if (ITimer0.attachInterruptInterval(TIEMPO_MICROSEG, funcion_timer) == false)
    Serial.println("Falla en ITimer0, seleccione otra frecuencia o timer");
}
void loop() {}