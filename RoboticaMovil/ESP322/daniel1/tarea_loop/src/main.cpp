#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.printf("setup se ejecuta en el nucleo %d con prioridad %d\n", xPortGetCoreID(),uxTaskPriorityGet(NULL));
}

void loop() {
  Serial.printf("loop se ejecuta en el nucleo %d con prioridad %d\n", xPortGetCoreID(),uxTaskPriorityGet(NULL));
  delay(1000);
}