#include <Arduino.h>
#include <macros.h>
#include <Motores.h>

Motores motoresDaniel;

void setup() {
  Serial.begin(115200);
  motoresDaniel.init(PIN_PWMA,PIN_AIN2,PIN_AIN1,PIN_STBY,PIN_BIN1,PIN_BIN2,PIN_PWMB);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  while(digitalRead(PIN_BOTON) == HIGH) ; // espera que se oprima el boton
  motoresDaniel.activa();
}

void loop() {
  delay(1000);
  motoresDaniel.fijaPWMs( 50, 50);
  delay(1000);
  motoresDaniel.fijaPWMs(-50,-50);
  delay(1000);
  motoresDaniel.desconecta();
  delay(10000);
  motoresDaniel.frena();
  while(true) ;  
}