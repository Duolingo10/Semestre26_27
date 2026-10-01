#include <Arduino.h>
#include <macros.h>

void fijaPWMs(int pwmI, int pwmD) {
  pwmI = pwmI >  255?  255 : pwmI; pwmD = pwmD >  255?  255 : pwmD;
  pwmI = pwmI < -255? -255 : pwmI; pwmD = pwmD < -255? -255 : pwmD;
  if(pwmI>=0){digitalWrite(PIN_AIN1,HIGH); digitalWrite(PIN_AIN2,LOW);} 
  else       {digitalWrite(PIN_AIN1,LOW);  digitalWrite(PIN_AIN2,HIGH); pwmI = -pwmI;}
  if(pwmD>=0){digitalWrite(PIN_BIN1,HIGH); digitalWrite(PIN_BIN2,LOW);} 
  else       {digitalWrite(PIN_BIN1,LOW);  digitalWrite(PIN_BIN2,HIGH); pwmD = -pwmD;}
  ledcWrite(PWM_CANAL_A, pwmI);    ledcWrite(PWM_CANAL_B, pwmD); 
}

void desconecta(void){
  fijaPWMs(255,255);
  digitalWrite(PIN_AIN1,LOW); digitalWrite(PIN_AIN2,LOW);
  digitalWrite(PIN_BIN1,LOW); digitalWrite(PIN_BIN2,LOW);
}

void frena(void){
  fijaPWMs(0,0);
  digitalWrite(PIN_AIN1,HIGH); digitalWrite(PIN_AIN2,HIGH);
  digitalWrite(PIN_BIN1,HIGH); digitalWrite(PIN_BIN2,HIGH);
}

void setup() {
  pinMode(PIN_LED,  OUTPUT);
  pinMode(PIN_BOTON,INPUT_PULLUP);
  pinMode(PIN_BIN1,OUTPUT);   pinMode(PIN_BIN2,OUTPUT); 
  pinMode(PIN_AIN1,OUTPUT);   pinMode(PIN_AIN2,OUTPUT);
  pinMode(PIN_PWMA,OUTPUT);   pinMode(PIN_PWMB,OUTPUT);
  pinMode(PIN_STBY,OUTPUT);
  digitalWrite(PIN_STBY,HIGH);
  ledcSetup(PWM_CANAL_A,PWM_FRECUENCIA, PWM_RESOLUCION);
  ledcSetup(PWM_CANAL_B,PWM_FRECUENCIA, PWM_RESOLUCION);
  ledcAttachPin(PIN_PWMA,PWM_CANAL_A);
  ledcAttachPin(PIN_PWMB,PWM_CANAL_B);
  fijaPWMs(0,0);
  digitalWrite(PIN_LED, HIGH);
  while(digitalRead(PIN_BOTON) == HIGH)  ; // espera a que se oprima el boton
  digitalWrite(PIN_LED, LOW);
}

void loop() {
  fijaPWMs(40,40);
  delay(2000);
  fijaPWMs(-40,-40);
  delay(2000);
  desconecta();  // intente empujar el robot, observe si opone resistencia
  delay(20000);
  frena();       // intente empujar el robot, observe si opone resistencia
  while(true) ;
}