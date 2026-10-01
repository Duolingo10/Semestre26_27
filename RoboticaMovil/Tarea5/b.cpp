#include <Arduino.h>

const int trigPin = 5;
const int echoPin = 18;
const int buzzerPin = 2; 

long duracion;
int distancia;

// Variables para controlar el zumbador
unsigned long tiempoAnterior = 0;
bool estadoBuzzer = LOW;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  
}

void loop() {
  // 1. Enviar pulso al sensor ultrasónico
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // 2. Leer eco con límite de tiempo
  duracion = pulseIn(echoPin, HIGH, 30000); 
  
  // Calcular distancia de forma interna
  if (duracion == 0) {
    distancia = 200; // Si falla, forzamos un valor fuera de rango
  } else {
    distancia = duracion * 0.034 / 2;
  }

  // 3. Alarma   
  unsigned long tiempoActual = millis();

  if (distancia > 0 && distancia <= 100) { 
    
    if (distancia <= 10) {
      // Muy cerca: Tono continuo
      digitalWrite(buzzerPin, HIGH);
      estadoBuzzer = HIGH; 
    } 
    else {
      // Pausa dinámica
      int tiempoEspera = distancia * 8; 
      int intervalo = estadoBuzzer ? 50 : tiempoEspera;
      
      if (tiempoActual - tiempoAnterior >= intervalo) {
        tiempoAnterior = tiempoActual;
        estadoBuzzer = !estadoBuzzer; 
        digitalWrite(buzzerPin, estadoBuzzer);
      }
    }
    
  } else {
    // Lejos: Silencio
    digitalWrite(buzzerPin, LOW);
    estadoBuzzer = LOW;
  }
  
  // Pequeña pausa para evitar rebotes acústicos del sensor
  delay(50);
}