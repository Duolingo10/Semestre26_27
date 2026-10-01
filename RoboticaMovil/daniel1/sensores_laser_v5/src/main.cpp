#include <Arduino.h>
#include <VL6180X.h> // Biblioteca VL6180X de Pololu
#include <macros.h>

VL6180X sensor[VL6180X_N];
int pin_shut[VL6180X_N_SHUT]={PIN_SHUT0,PIN_SHUT1,PIN_SHUT2,
                 PIN_SHUT3,PIN_SHUT4,PIN_SHUT5,PIN_SHUT6};
int  pin_int[VL6180X_N_INT] ={PIN_INT0,PIN_INT1,PIN_INT2,PIN_INT3};
int  escala[VL6180X_N]={1,1,1,2,1,1,1,1}; //solo frontal es 2
int  maxConvTime[VL6180X_N]={8,8,8,12,8,8,8,12};
bool laser_lento[VL6180X_N] = {false,false,false,true,true,true,true,true};
bool leer[VL6180X_N]={true,true,true,true,true,true,true,true}; 

void configura_sensor(VL6180X *sensor,byte direccion,byte k,byte ms) {
  delay(100); // Espera que termine la activacion del sensor (SHUT=HIGH)
  sensor->init();
  delay(10);
  sensor->configureDefault(); 
  sensor->setAddress(direccion);   // default 0x29
  sensor->setScaling(k);
  sensor->setTimeout(500);
  sensor->writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, ms); // en ms
  sensor->writeReg(VL6180X::SYSTEM__MODE_GPIO1, 0x10); // Habilita pin INT en LOW
  sensor->writeReg(VL6180X::SYSTEM__INTERRUPT_CONFIG_GPIO,0x04); // Cuando muestra lista
  sensor->writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x03); // borra interrupcion
  delay(10);
}

void setup() {
  int i;
  Serial.begin(115200);
  for (i=0; i < VL6180X_N_SHUT;i++) {
    pinMode(pin_shut[i], OUTPUT);  digitalWrite(pin_shut[i], LOW);
  }
  for (i=0; i < VL6180X_N_INT;i++) pinMode(pin_int[i], INPUT_PULLUP);
  Wire.begin(PIN_SDA,PIN_SCL,400000L);  // PIN_SDA, PIN_SCL, Frecuencia
  delay(2000);
  Serial.println(Wire.getClock());
  Serial.println("sensor 7"); Serial.flush();
  configura_sensor(&sensor[VL6180X_N-1], 0x31,escala[VL6180X_N-1],maxConvTime[VL6180X_N-1]); 
  for (i=0; i < VL6180X_N_SHUT;i++) {
    Serial.printf("sensor %d\n",i); Serial.flush();
    pinMode(pin_shut[i], INPUT);  
    configura_sensor(&sensor[i],0x32+i,escala[i],maxConvTime[i]);
  }
  configura_sensor(&sensor[VL6180X_N-1],0x29,escala[VL6180X_N-1],maxConvTime[VL6180X_N-1]);
  Serial.println("Sensores listos para medir"); Serial.flush();
}

void loop(){
  static bool laser_lentos_disp = true;
  static int i, mm[VL6180X_N]={0};
  bool fin_mediciones = false, 
       fin_sensor[VL6180X_N]={false,false,false,false,false,false,false,false};
  unsigned long t0, t1;
  t0 = millis();
  for(i=0; i < VL6180X_N; i++){
    if(leer[i] && (laser_lentos_disp || !laser_lento[i]))
      sensor[i].writeReg(VL6180X::SYSRANGE__START, 0x01);   // inicia medicion;
    fin_sensor[i] = laser_lento[i] && laser_lentos_disp? true : false;
  }
  delay(2);
  while(fin_mediciones == false) {
    // Sensores VL6180X con pin INT conectado al ESP32
    for(i=0; i < VL6180X_N_INT; i++) {
      if(leer[i] && (fin_sensor[i] == false) && (digitalRead(pin_int[i]) == LOW) ) { 
        mm[i] = sensor[i].readReg(VL6180X::RESULT__RANGE_VAL) * escala[i]; 
        sensor[i].writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x01);
        fin_sensor[i] = true;
      }
    }
    // Sensores VL6180X SIN pin INT conectado al ESP32
    for (i=VL6180X_N_INT; i < VL6180X_N; i++){
      if(leer[i] && (fin_sensor[i] == false) && 
        ((sensor[i].readReg(VL6180X::RESULT__INTERRUPT_STATUS_GPIO) & 0x04) != 0) ) { 
        mm[i] = sensor[i].readReg(VL6180X::RESULT__RANGE_VAL) * escala[i]; 
        sensor[i].writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x01);
        fin_sensor[i] = true;
        }
    }
    fin_mediciones = true;
    for(i=0; i < VL6180X_N; i++){
      if(leer[i] && fin_sensor[i] == false)
        fin_mediciones = false;
    }
    if(!fin_mediciones) delay(1);
  }
  laser_lentos_disp = !laser_lentos_disp;
  t1 = millis();
  for (i=0; i < VL6180X_N; i++) 
    Serial.printf("mm[%d]=%3d ",i, mm[i]);
  Serial.printf("ms=%d\n",t1-t0);
  Serial.flush();
}