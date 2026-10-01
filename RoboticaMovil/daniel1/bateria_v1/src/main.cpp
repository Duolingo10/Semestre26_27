#include <Arduino.h>
#include <macros.h>

float nivel_bateria(int pin_boton, float factor_conversion) {
  int n= 10;
  float volts = 0.0;
  adcAttachPin(pin_boton);  // prepara pin para lectura analogica
  delay(100);               // espera para estabilizar la lectura
  for(int i = 0; i < n; i++){
    volts += analogRead(pin_boton) * factor_conversion; 
    delay(1);
  }
  volts = volts / n;
  pinMode(pin_boton,INPUT_PULLUP);
  return volts;
}

void parpadear(int pin_led, int n) {
  int tiempo_ms = 500;
  for(int i = 0; i < n; i++){
    digitalWrite(pin_led, HIGH);
    delay(tiempo_ms);
    digitalWrite(pin_led, LOW);
    delay(tiempo_ms);
  }
}

void setup() {
  float volts=0.0;
  int n;

//  pinMode(PIN_STBY,    OUTPUT);       digitalWrite(PIN_STBY,LOW);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED,   OUTPUT);         digitalWrite(PIN_LED, HIGH);
  Serial.begin(115200);
  while(digitalRead(PIN_BOTON) == HIGH) ; // espera a que oprima el boton
  volts = nivel_bateria(PIN_BOTON, CONV_BAT); 
  digitalWrite(PIN_LED, LOW);
  n = (volts - 3.2)* 10;
  Serial.printf("volts = %f n = %d\n",volts, n);
  parpadear(PIN_LED, n);
}

void loop() {}