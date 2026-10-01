#include <Arduino.h>

volatile int v0 = 0, v1 = 0, v = 0;

void funcion_tarea0(void *parameter) {
  Serial.printf("funcion_tarea0 se ejecuta en nucleo %d con prioridad %d\n", xPortGetCoreID(), uxTaskPriorityGet(NULL));
  while (true){
    v = v-1;
    v0++;
  }
}

void funcion_tarea1(void *parameter) {
  Serial.printf("funcion_tarea1 se ejecuta en nucleo %d con prioridad %d\n", xPortGetCoreID(), uxTaskPriorityGet(NULL));
  while (true) {
    v = v+1;
    v1++;
  }
}

void setup() {
  TaskHandle_t tarea0, tarea1;
  Serial.begin(115200);
  xTaskCreatePinnedToCore(funcion_tarea0, "Tarea0", 10000, NULL, 1, &tarea0, 0); 
  xTaskCreatePinnedToCore(funcion_tarea1, "Tarea1", 10000, NULL, 1, &tarea1, 0);
}

void loop() {
  Serial.printf("v0 = %d v1 = %d v = %d\n", v0, v1, v); Serial.flush();
  delay(1000);
}