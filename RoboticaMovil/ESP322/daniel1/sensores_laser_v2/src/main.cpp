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
  sensor->stopContinuous();
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
  int i, mm[VL6180X_N]={0};
  bool fin_mediciones = false, fin_sensor[VL6180X_N]={false,false,false};
  unsigned long t0, t1;
  t0 = millis();
  for(i=0; i < VL6180X_N; i++) 
    sensor[i].writeReg(VL6180X::SYSRANGE__START, 0x01);   // inicia medicion;
  while(fin_mediciones == false) {
    for(i=0; i < VL6180X_N; i++) {
      if((fin_sensor[i] == false) && 
        ((sensor[i].readReg(VL6180X::RESULT__INTERRUPT_STATUS_GPIO) & 0x04) != 0) ) { 
        mm[i] = sensor[i].readReg(VL6180X::RESULT__RANGE_VAL); 
        sensor[i].writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x01);
        fin_sensor[i] = true;
      }
    }  
    fin_mediciones = true;
    for(i=0; i < VL6180X_N; i++){
      if(fin_sensor[i] == false)
        fin_mediciones = false;
    }
  }
  t1 = millis();
  mm[3] *= 2;  // correcion por factor escala = 2 en sensor Frontal
  for (i=0; i < VL6180X_N; i++) 
    Serial.printf("mm[%d]=%3d ",i, mm[i]);
  Serial.printf("ms=%d\n",t1-t0);
  Serial.flush();
  delay(1000);
}