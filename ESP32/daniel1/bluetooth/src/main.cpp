#include <Arduino.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  delay(3000); 
  SerialBT.begin("ESP32-Daniel");
  Serial.print("Esperando que la conexion BT se establezca: "); 
  Serial.flush();
  while(SerialBT.connected() == false){
     Serial.print(".");
     delay(500);
  }
  Serial.println(" LISTA"); Serial.flush();
  SerialBT.println("Robot Daniel ... listo");  SerialBT.flush();
}

void loop() {}
