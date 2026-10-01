#include <Arduino.h>
#include <macros.h>

void funcion_tarea1(void *parameter) {
  Serial.printf("funcion_tarea1 se ejecuta en nucleo %d con prioridad %d\n", xPortGetCoreID(), uxTaskPriorityGet(NULL));
  while (true)
    digitalWrite(PIN_LED, HIGH);
}

void setup() {
  Serial.begin(115200);
  char nombreTarea1[] = "Tarea1";
  int nucleo = 1, prioridad = 0, bytes_stack = 10000;
  void *argumentos = NULL;
  TaskHandle_t tarea1;

  pinMode(PIN_STBY, OUTPUT);  digitalWrite(PIN_STBY, LOW);
  pinMode(PIN_LED,  OUTPUT);
  xTaskCreatePinnedToCore(funcion_tarea1, nombreTarea1, bytes_stack, argumentos, prioridad, &tarea1, nucleo);
  delay(100);
}

void loop() {
  Serial.printf("loop se ejecuta en el nucleo %d con prioridad %d\n", xPortGetCoreID(), uxTaskPriorityGet(NULL));
  while (true) {
    digitalWrite(PIN_LED, LOW);
    delay(100);
  }
}