#include <Arduino.h>
#include <VL6180X.h> // Biblioteca VL6180X de Pololu
#include <macros.h>

VL6180X sensor[VL6180X_N];
int pin_shut[VL6180X_N_SHUT]={PIN_SHUT0,PIN_SHUT1,PIN_SHUT2,
                 PIN_SHUT3,PIN_SHUT4,PIN_SHUT5,PIN_SHUT6};

void configura_sensor(VL6180X *sensor, byte direccion, byte escala) {
  delay(100); // Espera que termine la activacion del sensor (SHUT=HIGH)
  sensor->init();
  delay(10);
  sensor->configureDefault(); 
  sensor->setAddress(direccion);   // default 0x29
  sensor->setScaling(escala);
  sensor->setTimeout(500);
  sensor->stopContinuous();   // stop continuous mode if already active
  delay(10);
}

void setup() {
  int i, escala;
  Serial.begin(115200);
  delay(2000);
  for (i=0; i < VL6180X_N_SHUT;i++) {
    pinMode(pin_shut[i], OUTPUT);  digitalWrite(pin_shut[i], LOW);
  }
  Wire.begin(PIN_SDA,PIN_SCL,100000L);  // PIN_SDA, PIN_SCL, Frecuencia
  Serial.println(Wire.getClock());
  Serial.println("sensor 7"); Serial.flush();
  configura_sensor(&sensor[VL6180X_N-1], 0x31,1); 
  for (i=0; i < VL6180X_N_SHUT;i++) {
    Serial.printf("sensor %d\n",i); Serial.flush();
    escala = (i==3)? 2: 1;  // sensor frontal con escala 2 (hasta 400mm)
    pinMode(pin_shut[i], INPUT);  configura_sensor(&sensor[i], 0x32+i, escala);
  }
  configura_sensor(&sensor[VL6180X_N-1], 0x29, 1); // direccion original
  Serial.println("Sensores listos para medir"); Serial.flush();
}

void loop(){
  int mm[VL6180X_N], i;
  unsigned long t0, t1;

  t0 = millis();
  for (i=0; i < VL6180X_N; i++) 
    mm[i] = sensor[i].readRangeSingleMillimeters();
  t1 = millis();
  for (i=0; i < VL6180X_N; i++) 
    Serial.printf("mm[%d]=%3d ",i, mm[i]);
  Serial.printf("ms=%d\n",t1-t0);
  delay(1000);
}
