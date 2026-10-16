#include <Arduino.h>

#define NUM_OP 20000000     // veinte millones

void setup() {
  Serial.begin(115200);
}

volatile byte  byte1 = 1,   byte2 = 2,    resultado_byte;
volatile int   entero1=10,  entero2 = 20, resultado_entero;
volatile float float1= 1.0, float2 = 2.0, resultado_float;

void loop() {
  unsigned long t0;   
  float seg;

  t0 = micros();    // Regresa el tiempo en microsegundos desde el inicio del programa 
  for(int i=0; i < NUM_OP; i++)
    resultado_byte = byte1 + byte2;
  seg = (micros() - t0) / 1.0E6;
  Serial.printf("%d sumas de bytes se hicieron en: %f segundos\n", NUM_OP, seg);

  t0 = micros();   
  for(int i=0; i < NUM_OP; i++)
    resultado_entero = entero1 + entero2;
  seg = (micros() - t0) / 1.0E6;
  Serial.printf("%d sumas de enteros se hicieron en: %f segundos\n", NUM_OP, seg);

  t0 = micros();   
  for(int i=0; i < NUM_OP; i++)
    resultado_float = float1 * float2;
  seg = (micros() - t0) / 1.0E6;
  Serial.printf("%d productos de floats se hicieron en: %f segundos\n", NUM_OP, seg);
  delay(10000);
}