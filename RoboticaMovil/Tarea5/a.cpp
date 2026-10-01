#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
const int ledPin = 2; 

void setup() {
  // Configurar el pin del LED como salida
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Iniciar apagado
  
  // Iniciar el Bluetooth con el nombre del dispositivo
  SerialBT.begin("Robot_ESP32"); 
}

void loop() {
  // Verificar si llegan datos por la conexión Bluetooth
  if (SerialBT.available()) {
    
    // Leer la cadena de texto hasta detectar un salto de línea
    String comando = SerialBT.readStringUntil('\n');
    
    // Limpiar caracteres invisibles (como \r o espacios extra de la app)
    comando.trim(); 

    // Encender o apagar el LED según el texto recibido
    if (comando == "led 1") {
      digitalWrite(ledPin, HIGH);
    } 
    else if (comando == "led 0") {
      digitalWrite(ledPin, LOW);
    }
  }
}