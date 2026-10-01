#include <Arduino.h>

SemaphoreHandle_t semaforo1;
volatile int v0 = 0, v1 = 0, v = 0;

void funcion_tarea0(void *parameter) {
  int v_local;
  xSemaphoreTake(semaforo1, portMAX_DELAY);
  Serial.printf("funcion_tarea0 se ejecuta en nucleo %d con prioridad %d\n", xPortGetCoreID(), uxTaskPriorityGet(NULL));
  xSemaphoreGive(semaforo1);
  while (true){
    xSemaphoreTake(semaforo1, portMAX_DELAY);
    v_local = v;
    delayMicroseconds(10*1000); 
    v = v_local - 1;
    v0++;
    xSemaphoreGive(semaforo1);
    delay(1);
  }
}

void funcion_tarea1(void *parameter) {
  int v_local;
  xSemaphoreTake(semaforo1, portMAX_DELAY);
  Serial.printf("funcion_tarea1 se ejecuta en nucleo %d con prioridad %d\n", xPortGetCoreID(), uxTaskPriorityGet(NULL));
  xSemaphoreGive(semaforo1);
  while (true) {
    xSemaphoreTake(semaforo1, portMAX_DELAY);
    v_local = v;
    delayMicroseconds(10*1000); 
    v = v_local + 1;
    v1++;
    xSemaphoreGive(semaforo1);
    delay(1);
  }
}

void setup() {
  Serial.begin(115200);
  semaforo1 = xSemaphoreCreateBinary();
  xSemaphoreGive(semaforo1);
  xTaskCreatePinnedToCore(funcion_tarea0, "Tarea0", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(funcion_tarea1, "Tarea1", 10000, NULL, 1, NULL, 1);
}

void loop() {
  xSemaphoreTake(semaforo1, portMAX_DELAY);
  Serial.printf("v0 = %5d v1 = %5d v1-v0 = %5d v = %5d\n", v0, v1, v1-v0, v); 
  Serial.flush();
  xSemaphoreGive(semaforo1);
 delay(1000);
}
