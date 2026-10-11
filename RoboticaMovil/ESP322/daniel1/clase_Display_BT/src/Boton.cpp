#include <Arduino.h>
#include <Boton.h>

bool Boton::init(int boton, int led){
  pin_boton = boton; pin_led = led; 
  pinMode(pin_boton,INPUT_PULLUP);
  pinMode(pin_led, OUTPUT);
  digitalWrite(pin_led,LOW);
  preferencias.begin("Daniel1", true); // abierto para lectura
  factor_conversion_volts = preferencias.getFloat("FC", 0);
  preferencias.end();
  if(factor_conversion_volts == 0.0) factor_conversion_volts = CONV_BAT;
  Serial.printf("Factor de conversion a volts = %f\n",factor_conversion_volts); Serial.flush();
  return true;
}

/* Regresa un numero: -1, 0, 1, 2, ... max
   -1 = Se oprimio el boton desde el arranque del microcontrolador
   0  = No se oprimio el boton en max encendidos del led
   1  = Se oprimio el boton cuando el led encendio por primera vez 
   2  = Se oprimio el boton cuando el led encendio por segunda vez */
int Boton::leeOpcion(int num_opciones){
  int tiempo_encendido = 2000, tiempo_apagado = 1000; //tiempo en ms
  unsigned long t0;
  int respuesta = 0;
  delay(tiempo_apagado);
  if(digitalRead(pin_boton) == LOW)  // Boton oprimido
    respuesta = -1;
  for(int i = 1; (i <= num_opciones) && (respuesta == 0); i++){
    digitalWrite(pin_led,HIGH);
    t0 = millis();
    while (millis() - t0 < tiempo_apagado){
      if(digitalRead(pin_boton) == LOW){ // Boton oprimido
        digitalWrite(pin_led, LOW);
        respuesta = i;
      }
    }
    digitalWrite(pin_led, LOW);
    delay(tiempo_apagado);
  }
  delay(tiempo_apagado);
  return respuesta;
}

void Boton::esperaEstado(int edo_fin){
  int tiempo_parpadeo=40, edo = HIGH;
  unsigned long t0;
  digitalWrite(pin_led, HIGH);
  t0 = millis();
  while(digitalRead(pin_boton) != edo_fin){
    digitalWrite(pin_led, edo);
    if(millis() - t0 >= tiempo_parpadeo){
      edo = (edo == HIGH)? LOW : HIGH;
      t0 = millis();
    }
  }
  digitalWrite(pin_led, LOW);
}

void Boton::parpadear(int veces){
  int tiempo_ms = 500;
  delay(tiempo_ms*4);
  for(int i = 0; i < veces; i++){
    digitalWrite(pin_led, HIGH);
    delay(tiempo_ms);
    digitalWrite(pin_led, LOW);
    delay(tiempo_ms);
  }
}

float Boton::nivelBateria(int *lectura){
  float volts=0.0;
  int medicion=0, max_lecturas=5;
  adcAttachPin(pin_boton);
  delay(100);
  for(int i = 0; i < max_lecturas; i++){
    delay(1);
    medicion += analogRead(pin_boton); 
  }
  pinMode(pin_boton,INPUT_PULLUP);
  medicion /= max_lecturas;
  volts = ((float) medicion * factor_conversion_volts);
  *lectura = medicion;
  return volts;
}

float Boton::bateria(void){
  int medicion, n;
  float volts;
  esperaOprimir();
  volts = nivelBateria(&medicion);
  n = (volts - 3.2)* 10;
  parpadear(n);
  esperaLiberar();
  Serial.printf("medicion = %d, volts = %f\n", medicion, volts); Serial.flush();
  return volts;
}

float Boton::calibra_bat(void) {
  int medicion, n; 
  float volts;
  esperaOprimir();
  nivelBateria(&medicion);
  esperaLiberar();
  Serial.printf("medicion = %d Ingrese el voltaje de la bateria (float):", medicion); 
  Serial.flush();
  Serial.setTimeout(60000); // Espera hasta por 60s (default 1000ms)
  volts = Serial.parseFloat();
  Serial.printf("\nEscribio %f\n", volts);
  factor_conversion_volts = volts /(float) medicion;
  Serial.printf("Factor de conversion = %f\n",factor_conversion_volts); Serial.flush();
  preferencias.begin("Daniel1", false); //abierto para lectura - escritura
  preferencias.putFloat("FC",factor_conversion_volts);
  preferencias.end();  
  return volts;
}

void Boton::ledON(void)         { digitalWrite(pin_led, HIGH); }
void Boton::ledOFF(void)        { digitalWrite(pin_led, LOW);  }
void Boton::esperaOprimir(void ){ esperaEstado(LOW);           } 
void Boton::esperaLiberar(void) { esperaEstado(HIGH);          }