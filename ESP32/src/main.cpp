#include <Arduino.h>

const int trigPin = 5;
const int echoPin = 18;
const int buzzerPin = 2; 

long duracion;
int distancia;

// Variables para controlar el zumbador sin detener el programa (millis)
unsigned long tiempoAnterior = 0;
bool estadoBuzzer = LOW;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  
  Serial.begin(115200);
}

void loop() {
  // 1. Enviar pulso al sensor ultrasónico
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // 2. Leer eco con un timeout de 30000 microsegundos (~5 metros).
  // Esto evita que el ESP32 se congele esperando un eco que nunca llega.
  duracion = pulseIn(echoPin, HIGH, 30000); 
  
  if (duracion == 0) {
    distancia = 200; // Si no hay eco, forzamos un valor fuera de rango
  } else {
    distancia = duracion * 0.034 / 2;
  }

  // 3. Obtener el tiempo actual del cronómetro del ESP32
  unsigned long tiempoActual = millis();

  // 4. Lógica de la alarma (No bloqueante)
  if (distancia > 0 && distancia <= 150) { 
    
    if (distancia <= 10) {
      // Peligro inminente: Tono continuo
      digitalWrite(buzzerPin, HIGH);
      estadoBuzzer = HIGH; // Sincronizamos la variable
    } 
    else {
      // Calculamos la pausa dinámica
      int tiempoEspera = distancia * 8; 
      
      // Determinamos si el zumbador debe estar encendido (50ms) o apagado (tiempoEspera)
      int intervalo = estadoBuzzer ? 50 : tiempoEspera;
      
      // Si ya pasó el tiempo necesario, cambiamos el estado del zumbador
      if (tiempoActual - tiempoAnterior >= intervalo) {
        tiempoAnterior = tiempoActual;
        estadoBuzzer = !estadoBuzzer; // Invertir el estado (encender si estaba apagado, o viceversa)
        digitalWrite(buzzerPin, estadoBuzzer);
      }
    }
    
  } else {
    // Fuera de peligro: Silencio
    digitalWrite(buzzerPin, LOW);
    estadoBuzzer = LOW;
  }
  
  // Una pausa mínima de 50ms para no saturar el sensor ultrasónico de rebotes acústicos
  delay(50);
}