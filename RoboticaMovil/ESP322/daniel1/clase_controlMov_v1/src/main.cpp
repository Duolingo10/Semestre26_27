#include <Arduino.h>
#include <macros.h>
#include <DISPLAY_BT.h>
#include <ControlMov.h>

ControlMov movDaniel;

void setup(){
  Serial.begin(115200);
  delay(2000);
  while(DISPLAY_BT::init_SSD1306()==false) delay(1000);
  movDaniel.initMotores(PIN_PWMA,PIN_AIN2,PIN_AIN1,PIN_STBY,PIN_BIN1,PIN_BIN2,PIN_PWMB);
  movDaniel.initEncoders(PIN_ENCB_IZQ, PIN_ENCA_IZQ, PIN_ENCB_DER, PIN_ENCA_DER);
  movDaniel.activaMotores();
}

void loop() {
  movDaniel.calibra_PWM_motores();
  movDaniel.leeDatosFlash();
  while(true) ;
}