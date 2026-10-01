#include <Arduino.h>
#include <macros.h>
#include <VL6180X_robot.h>

VL6180X_robot sensoresDaniel;

void setup() {
  Serial.begin(115200);  delay(3000);
  sensoresDaniel.init(PIN_SHUT0,PIN_SHUT1,PIN_SHUT2,PIN_SHUT3,PIN_SHUT4,PIN_SHUT5,
                      PIN_SHUT6,PIN_INT0, PIN_INT1, PIN_INT2, PIN_INT3);
  sensoresDaniel.iniciaTareaLecturasContinuas();
  delay(1000); // tiempo para adquirir las primeras lecturas
}

int mm[VL6180X_N]={0}, n, n0; 
float mmF[VL6180X_N]={0.0};
unsigned long t_ms;

void loop() {
  n0= VL6180X_robot::lecturasSensores(mm, mmF, &t_ms);
  delay(1000);    
  n = VL6180X_robot::lecturasSensores(mm, mmF, &t_ms);
  Serial.printf("mmI=%3d mmD=%3d n-n0=%d\n",mm[1],mm[5],n-n0);
  Serial.printf(" n  mmI  (f) mmD  (f)\n");
  for(int i=0; i < 50; i++) {
    n = VL6180X_robot::lecturasSensores(mm, mmF, &t_ms);
    Serial.printf("%3d,%3d,%4.1f,%3d,%4.1f\n",
     n,mm[1],mmF[1],mm[5],mmF[5]);
  }
  while(true) ;
}