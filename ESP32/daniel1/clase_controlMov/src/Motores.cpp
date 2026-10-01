#include <Arduino.h>
#include <Motores.h>
#include <macros.h>

bool Motores::init(int PWMA, int AIN2, int AIN1, int STBY, int BIN1, int BIN2, int PWMB){
  pin_PWMA=PWMA; pin_AIN2=AIN2; pin_AIN1=AIN1; pin_STBY=STBY; pin_BIN1=BIN1; pin_BIN2=BIN2; pin_PWMB=PWMB;
  pinMode(pin_BIN1,OUTPUT);   pinMode(pin_BIN2,OUTPUT); 
  pinMode(pin_AIN1,OUTPUT);   pinMode(pin_AIN2,OUTPUT);
  pinMode(pin_PWMA,OUTPUT);   pinMode(pin_PWMB,OUTPUT);
  pinMode(pin_STBY,OUTPUT);
  digitalWrite(pin_STBY,LOW);
  ledcSetup(PWM_CANAL_A,PWM_FRECUENCIA, PWM_RESOLUCION);
  ledcSetup(PWM_CANAL_B,PWM_FRECUENCIA, PWM_RESOLUCION);
  ledcAttachPin(pin_PWMA,PWM_CANAL_A);
  ledcAttachPin(pin_PWMB,PWM_CANAL_B);
  fijaPWMs(0,0);
  return true;
}

void Motores::activa(void) {  
  digitalWrite(pin_STBY, HIGH); 
}

void Motores::fijaDirs(int dir_izq, int dir_der){
  if(dir_izq>=0){digitalWrite(pin_AIN1,HIGH); digitalWrite(pin_AIN2,LOW);} 
  else          {digitalWrite(pin_AIN1,LOW);  digitalWrite(pin_AIN2,HIGH);}
  if(dir_der>=0){digitalWrite(pin_BIN1,HIGH); digitalWrite(pin_BIN2,LOW);} 
  else          {digitalWrite(pin_BIN1,LOW);  digitalWrite(pin_BIN2,HIGH);}
}

void Motores::fijaPWMs(int pwmI, int pwmD){
  pwmI = pwmI >  255?  255 : pwmI; pwmD = pwmD >  255?  255 : pwmD;
  pwmI = pwmI < -255? -255 : pwmI; pwmD = pwmD < -255? -255 : pwmD;
  fijaDirs(pwmI, pwmD);
  pwmI = abs(pwmI);          pwmD = abs(pwmD);
  ledcWrite(PWM_CANAL_A, pwmI);    ledcWrite(PWM_CANAL_B, pwmD); 
}

void Motores::desconecta(void){
  fijaPWMs(255,255);
  digitalWrite(pin_AIN1,LOW); digitalWrite(pin_AIN2,LOW);
  digitalWrite(pin_BIN1,LOW); digitalWrite(pin_BIN2,LOW);
}

void Motores::frena(void){
  fijaPWMs(0,0);
  digitalWrite(pin_AIN1,HIGH); digitalWrite(pin_AIN2,HIGH);
  digitalWrite(pin_BIN1,HIGH); digitalWrite(pin_BIN2,HIGH);
}