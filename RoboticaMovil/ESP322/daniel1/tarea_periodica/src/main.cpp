#include <Arduino.h>
#include <macros.h>

void funcion_tarea(void *parameter) {
  TickType_t ticks0, inc_ms = 1000;
  int estado = 0;
  Serial.printf("funcion_tarea corre en nucleo %d con prioridad %d\n",xPortGetCoreID(),uxTaskPriorityGet(NULL));
  ticks0 = xTaskGetTickCount(); 
  while(true){
    xTaskDelayUntil(&ticks0, inc_ms);
    Serial.println(millis());
    digitalWrite(PIN_LED,estado);
    estado = !estado;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_STBY,OUTPUT); digitalWrite(PIN_STBY,LOW);
  pinMode(PIN_LED, OUTPUT);
  xTaskCreatePinnedToCore(funcion_tarea, "Tarea", 10000, NULL,1, NULL, 1);
}

void loop() {}