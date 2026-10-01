#include <Arduino.h>
#include <macros.h>

/* Regresa un numero: -1, 0, 1, 2, ... max
   -1 = Se oprimio el boton desde el arranque del microcontrolador
   0  = No se oprimio en boton en max encendidos del led
   1  = Se oprimio el boton cuando el led encendio por primera vez 
   2  = Se oprimio el boton cuando el led encendio por segunda vez 
   3  = Se oprimio el boton cuando el led encendio por tercera vez */
int lee_boton(int pin_boton, int pin_led, int max){
  unsigned long t0;
  int respuesta = 0;
  if(digitalRead(pin_boton) == LOW)  // Boton oprimido
    respuesta = -1;
  for(int i = 1; (i <= max) && (respuesta == 0); i++){
    digitalWrite(pin_led,HIGH);
    t0 = millis();
    while (millis() - t0 < 2000){
      if(digitalRead(pin_boton) == LOW){ // Boton oprimido
        digitalWrite(pin_led, LOW);
        respuesta = i;
      }
    }
    digitalWrite(pin_led, LOW);
    delay(1000);
  }
  return respuesta;
}

int opcion;

void setup() {
  Serial.begin(115200);
  delay(2000);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED,   OUTPUT);
  opcion = lee_boton(PIN_BOTON, PIN_LED, 3);
  Serial.printf("la lectura del boton es: %d\n",opcion);
}

void loop() {}
