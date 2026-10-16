#include <Arduino.h>
#include <VL6180X.h> // Biblioteca VL6180X de Pololu
#include <macros.h>

VL6180X sensor;

void setup() {
  Serial.begin(115200);
  delay(2000);
  pinMode(PIN_SHUT0, OUTPUT);  digitalWrite(PIN_SHUT0, LOW);
  pinMode(PIN_SHUT1, OUTPUT);  digitalWrite(PIN_SHUT1, LOW);
  pinMode(PIN_SHUT2, OUTPUT);  digitalWrite(PIN_SHUT2, LOW);
  pinMode(PIN_SHUT3, OUTPUT);  digitalWrite(PIN_SHUT3, LOW);
  pinMode(PIN_SHUT4, OUTPUT);  digitalWrite(PIN_SHUT4, LOW);
  pinMode(PIN_SHUT5, OUTPUT);  digitalWrite(PIN_SHUT5, LOW);
  pinMode(PIN_SHUT6, OUTPUT);  digitalWrite(PIN_SHUT6, LOW);
  Wire.begin(PIN_SDA,PIN_SCL,100000L);  // PIN_SDA, PIN_SCL, Frecuencia
  Serial.println(Wire.getClock());
  sensor.init();
  sensor.configureDefault(); 
//  sensorA.setAddress(0x31);   // direccion inicial 0x29
  sensor.setScaling(1);
  sensor.setTimeout(500);
  sensor.stopContinuous();   // detiene el modo continuo
  delay(100);
}

void loop(){
  int mm;
  mm = sensor.readRangeSingleMillimeters();
  Serial.printf("mm = %d\n", mm);
  delay(100);
}