#include <Arduino.h>
#include <Wire.h>
 
void setup() {
  Wire.begin(21, 22, 100000); // SDA=pin21, SCL=pin22, frecuencia=100000bps
  Serial.begin(115200);
  Serial.println(Wire.getClock());
  Serial.println("Escanner I2C");
}
 
void loop() {
  byte error, address;
  int n = 0;
  Serial.println("Detectando ...");
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Dispositivo I2C 0x");
      if (address<16) Serial.print("0");
      Serial.println(address,HEX);
      n++;
    }
  }
  if (n == 0) 
    Serial.println("No de detectaron dispositivos I2C \n");
  delay(5000);          
}
