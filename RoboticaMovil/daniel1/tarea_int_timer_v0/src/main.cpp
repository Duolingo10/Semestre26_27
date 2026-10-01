#include <Arduino.h>
#include <macros.h>

hw_timer_t *timer0 = NULL;
volatile int tiempo_ms =0;
SemaphoreHandle_t semaforo1;

void IRAM_ATTR funcion_timer(){
  static int estado = true;
  digitalWrite(PIN_LED, estado);
  estado = !estado;
  tiempo_ms = millis();
  xSemaphoreGive(semaforo1);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_STBY,OUTPUT); digitalWrite(PIN_STBY,LOW);
  pinMode(PIN_LED, OUTPUT);
  semaforo1 = xSemaphoreCreateBinary();
  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &funcion_timer, true);
  timerAlarmWrite(timer0, 1000000, true);
  timerAlarmEnable(timer0);
}

void loop() {
  xSemaphoreTake(semaforo1, portMAX_DELAY);
  Serial.println(tiempo_ms);
}