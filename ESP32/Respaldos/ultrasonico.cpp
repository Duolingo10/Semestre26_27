#include <Arduino.h>
// Definición de los pines para ESP32
const int trigPin = 5;
const int echoPin = 18;
const int buzzerPin = 19; // Puedes cambiar a GPIO 2 si prefieres usar el LED azul integrado en la placa

// Variables para el cálculo de distancia
long duracion;
int distancia;

void setup() {
  // Configuración de pines
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  
  // Iniciar comunicación serial a 115200 baudios (estándar para ESP32)
  Serial.begin(115200);
}

void loop() {
  // 1. Limpiar el pin TRIG
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // 2. Enviar pulso de disparo de 10 microsegundos
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // 3. Leer el eco
  duracion = pulseIn(echoPin, HIGH);
  
  // 4. Calcular distancia
  distancia = duracion * 0.034 / 2;
  
  // Mostrar la distancia en el Monitor Serie
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // 5. Lógica de la alarma
  if (distancia > 0 && distancia <= 150) { 
    
    if (distancia <= 10) {
      // Menos de 10cm: Tono continuo de precaución
      digitalWrite(buzzerPin, HIGH);
    } 
    else {
      // Entre 10cm y 150cm: Pitidos proporcionales
      int tiempoEspera = distancia * 8; 
      
      digitalWrite(buzzerPin, HIGH);
      delay(50); 
      digitalWrite(buzzerPin, LOW);
      delay(tiempoEspera); 
    }
    
  } else {
    // Fuera de peligro: Silencio
    digitalWrite(buzzerPin, LOW);
  }
}