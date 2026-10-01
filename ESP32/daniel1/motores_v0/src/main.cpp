#include <Arduino.h>
#include <macros.h>

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED,OUTPUT);
  pinMode(PIN_BOTON,INPUT_PULLUP);
  pinMode(PIN_BIN1,OUTPUT);   pinMode(PIN_BIN2,OUTPUT); 
  pinMode(PIN_AIN1,OUTPUT);   pinMode(PIN_AIN2,OUTPUT);
  pinMode(PIN_PWMA,OUTPUT);   pinMode(PIN_PWMB,OUTPUT);
  pinMode(PIN_STBY,OUTPUT);
  digitalWrite(PIN_AIN1,LOW); digitalWrite(PIN_AIN2,LOW);
  digitalWrite(PIN_BIN2,LOW); digitalWrite(PIN_BIN1,LOW);
  digitalWrite(PIN_STBY,HIGH);
  ledcSetup(PWM_CANAL_A,PWM_FRECUENCIA, PWM_RESOLUCION);
  ledcSetup(PWM_CANAL_B,PWM_FRECUENCIA, PWM_RESOLUCION);
  ledcAttachPin(PIN_PWMA,PWM_CANAL_A);
  ledcAttachPin(PIN_PWMB,PWM_CANAL_B);
  ledcWrite(PWM_CANAL_A, 0);    ledcWrite(PWM_CANAL_B, 0);  // Fija PWMs = 0
  digitalWrite(PIN_LED, HIGH);
  while(digitalRead(PIN_BOTON) == HIGH)  ; // espera a que se oprima el boton
  digitalWrite(PIN_LED, LOW);
}

void loop() {
  digitalWrite(PIN_AIN1,HIGH); digitalWrite(PIN_AIN2,LOW);
  digitalWrite(PIN_BIN1,HIGH); digitalWrite(PIN_BIN2,LOW); ;
  ledcWrite(PWM_CANAL_A, 40);  ledcWrite(PWM_CANAL_B, 40); 
  delay(2000);
  digitalWrite(PIN_AIN1,LOW); digitalWrite(PIN_AIN2,HIGH);
  digitalWrite(PIN_BIN1,LOW); digitalWrite(PIN_BIN2,HIGH); 
  ledcWrite(PWM_CANAL_A, 40); ledcWrite(PWM_CANAL_B, 40); 
  delay(2000);
  digitalWrite(PIN_AIN1,LOW); digitalWrite(PIN_AIN2,LOW);
  digitalWrite(PIN_BIN2,LOW); digitalWrite(PIN_BIN1,LOW);
  ledcWrite(PWM_CANAL_A, 0);  ledcWrite(PWM_CANAL_B, 0); 
  while(true) ;
}