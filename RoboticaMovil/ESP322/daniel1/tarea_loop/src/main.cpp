#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.printf("setup se ejecuta en el nucleo %d con prioridad %d\n", xPortGetCoreID(),uxTaskPriorityGet(NULL));
}

void loop() {
  Serial.printf("loop se ejecuta en el nucleo %d con prioridad %d\n", xPortGetCoreID(),uxTaskPriorityGet(NULL));
  delay(1000);
}


/*Entonces el orden por carriles es el siguiente:
Carril 1:30 HZ
Carril 2:20 HZ
Carril 3:10 HZ
CArril 4:30 HZ
Carril 5:20 HZ
Carril 6:10 HZ
Carril 7:30 HZ
Carril 8:20 HZ

Por ejemplo el carril 1: Empieza en 30 HZ y despues de un tiempo cambia a 20 HZ y luego a 10 HZ, despues vuelve a 30 HZ y asi sucesivamente.
*/