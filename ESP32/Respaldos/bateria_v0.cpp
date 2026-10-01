
#include <Arduino.h>
#include <macros.h>

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.printf("%d, %d\n",analogRead(PIN_X),analogRead(PIN_Y));  
  delay(100); 
}