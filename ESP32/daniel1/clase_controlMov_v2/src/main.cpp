#include <Arduino.h>
#include <macros.h>
#include <DISPLAY_BT.h>
#include <ControlMov.h>
#include <Boton.h>

ControlMov movDaniel;
Boton      botonDaniel;

void setup(){
  Serial.begin(115200);
  delay(2000);
  botonDaniel.init(PIN_BOTON,PIN_LED);
  while(DISPLAY_BT::init_SSD1306()==false) delay(1000);
  movDaniel.initMotores(PIN_PWMA,PIN_AIN2,PIN_AIN1,PIN_STBY,PIN_BIN1,PIN_BIN2,PIN_PWMB);
  movDaniel.initEncoders(PIN_ENCB_IZQ, PIN_ENCA_IZQ, PIN_ENCB_DER, PIN_ENCA_DER);
  movDaniel.activaMotores();
  botonDaniel.esperaOprimir();
}

void loop() {
  int pasos_ref, pasos, mm, grados, vel=10;
  float acel=1; // aceleracion
  char texto[80];
  movDaniel.avanza_vel(20,acel);  // avanza con vel=10,
  delay(1000);                     // por 2 segundos
  movDaniel.para(); delay(3000);   // y para
  mm = 200;
  pasos_ref = MM_A_PASOS(mm);
  pasos = movDaniel.avanza_pasos(pasos_ref,vel,acel);
  sprintf(texto,"av %d\npasos=%d",pasos_ref,pasos);
  DISPLAY_BT::print(texto,2); delay(3000);
  pasos = movDaniel.retrocede_pasos(pasos_ref,vel,acel);
  sprintf(texto,"ret %d\npasos=%d",pasos_ref,pasos);
  DISPLAY_BT::print(texto,2); delay(3000);
  grados = 90;
  pasos_ref = GRADOS_A_PASOS(grados);
  pasos = movDaniel.gira_pasos(pasos_ref,vel,acel);
  sprintf(texto,"giro %d\npasos=%d",pasos_ref, pasos);
  DISPLAY_BT::print(texto,2); delay(3000);
  grados = -90;
  pasos_ref = GRADOS_A_PASOS(grados);
  pasos = movDaniel.gira_pasos(pasos_ref,vel,acel);
  sprintf(texto,"giro %d\npasos=%d",pasos_ref, pasos);
  DISPLAY_BT::print(texto,2);
  while(true) ;
}