#include <Arduino.h>
#include <VL6180X.h> // Biblioteca VL6180X de Pololu
#include <macros.h>

VL6180X sensor[VL6180X_N];
int pin_shut[VL6180X_N_SHUT]={PIN_SHUT0,PIN_SHUT1,PIN_SHUT2,
                 PIN_SHUT3,PIN_SHUT4,PIN_SHUT5,PIN_SHUT6};
int pin_int[VL6180X_N_INT] ={PIN_INT0,PIN_INT1,PIN_INT2,PIN_INT3};

void configura_sensor(VL6180X *sensor, byte direccion, byte escala) {
  delay(100); // Espera que termine la activacion del sensor (SHUT=HIGH)
  sensor->init();
  delay(10);
  sensor->configureDefault(); 
  sensor->setAddress(direccion);   // default 0x29
  sensor->setScaling(escala);
  sensor->setTimeout(500);
  sensor->writeReg(VL6180X::SYSTEM__MODE_GPIO1, 0x10); // Habilita pin INT en LOW
  sensor->writeReg(VL6180X::SYSTEM__INTERRUPT_CONFIG_GPIO,0x04); // Cuando muestra lista
  sensor->writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x03); // borra interrupcion
  delay(10);
}

void setup() {
  int i, escala;
  Serial.begin(115200);
  delay(2000);
  for (i=0; i < VL6180X_N_SHUT;i++) {
    pinMode(pin_shut[i], OUTPUT);  digitalWrite(pin_shut[i], LOW);
  }
  for (i=0; i < VL6180X_N_INT;i++) pinMode(pin_int[i], INPUT_PULLUP);
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
  int mm, edo_pin;
  ulong time0, time1;

  edo_pin = digitalRead(pin_int[0]);
  Serial.printf("antes INT =%d\n",edo_pin);
  time0=millis();
  sensor[0].writeReg(VL6180X::SYSRANGE__START, 0x01); // inicia medicion
  do{
    delay(1);
  } while(digitalRead(pin_int[0]) == edo_pin);
  time1 = millis();
  Serial.printf("+%d ms despues INT =%d\n", time1 - time0, digitalRead(pin_int[0]));
  mm = sensor[0].readReg(VL6180X::RESULT__RANGE_VAL);
  sensor[0].writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x01);
  time1 = millis();
  Serial.printf("medicion=%d tiempo=%d ms\n", mm, time1 - time0);
  delay(2000);

}