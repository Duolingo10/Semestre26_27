#include <Arduino.h>
#include <macros.h>
#include <DISPLAY_BT.h>
#include <ControlMov.h>
#include <Boton.h>

ControlMov movDaniel;
Boton      botonDaniel;

void setup(){
  char robot_ID[] = "Daniel";
  Serial.begin(115200);
  delay(2000);
  botonDaniel.init(PIN_BOTON,PIN_LED);
  while(DISPLAY_BT::init_SSD1306()==false) delay(1000);
  movDaniel.initMotores(PIN_PWMA,PIN_AIN2,PIN_AIN1,PIN_STBY,PIN_BIN1,PIN_BIN2,PIN_PWMB);
  movDaniel.initEncoders(PIN_ENCB_IZQ, PIN_ENCA_IZQ, PIN_ENCB_DER, PIN_ENCA_DER);
  movDaniel.activaMotores();
  DISPLAY_BT::init_BT(robot_ID);
  botonDaniel.esperaOprimir();
  delay(1000);
}

void loop() {
  DISPLAY_BT::init_BT("DANIEL1"); 
  movDaniel.estimaKpKdKi();
  DISPLAY_BT::print("Fin\nestimacion",2);
  delay(3000);
  DISPLAY_BT::print("Prueba\navanza 300",2);
  movDaniel.avanza_pasos(MM_A_PASOS(300),20,1.0f); // avanza 300mm
  while(true) ;
}