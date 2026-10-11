#include <Arduino.h>
#include <Preferences.h>
#include <macros.h>

Preferences preferencias;
float factor_conversion; // para convertir lectura a volts de bateria

void calibra_bat(int pin_boton, int pin_led) {
  int medicion;
  float volts, factor_conv;
  digitalWrite(pin_led, HIGH);
  Serial.println("Oprime el boton para medir la bateria"); Serial.flush();
  while(digitalRead(pin_boton)== HIGH) ; // espera a que se oprima el boton
  adcAttachPin(pin_boton);  // prepara pin para lectura analogica
  delay(100);               // espera para estabilizar la lectura
  { int suma=0, n=10, i;
    for(i=0; i < n; i++) {
      suma += analogRead(pin_boton);
      delay(1);
    }
   medicion = suma /n;
  }
  pinMode(pin_boton,INPUT_PULLUP);
  digitalWrite(pin_led, LOW);
  Serial.printf("medicion = %d Ingrese el voltaje de la bateria (float):", medicion); 
  Serial.flush();
  Serial.setTimeout(60000); // Espera hasta por 60s (default 1000ms)
  volts = Serial.parseFloat();
  Serial.printf("\nEscribio %f\n", volts);
  factor_conv = volts /(float) medicion;
  Serial.printf("Factor de conversion = %f\n",factor_conv); Serial.flush();
  preferencias.begin("Daniel", false); //abierto para lectura - escritura
  preferencias.putFloat("FC",factor_conv);
  preferencias.end();  
}

void setup() {
  pinMode(PIN_STBY,  OUTPUT);        digitalWrite(PIN_STBY, LOW);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED,   OUTPUT);
  Serial.begin(115200);
  preferencias.begin("Daniel", true); // abierto para lectura
  factor_conversion = preferencias.getFloat("FC", 0.0);
  preferencias.end();
  if(factor_conversion > 0.0) 
    Serial.printf("Se recupero el factor de conversion %f\n", factor_conversion); 
  else {
    calibra_bat(PIN_BOTON, PIN_LED);
    Serial.println("Reset en 5 segundos");
    delay(5000);
    ESP.restart();
  }
}

void loop() { }
