#include <Arduino.h>
#include <Boton.h>
#include <DISPLAY_BT.h>

Boton botonDaniel;

void setup() {
  char texto[200];
  Serial.begin(115200); 
  delay(2000);
  botonDaniel.init(PIN_BOTON, PIN_LED);
  DISPLAY_BT::init_SSD1306();
}

void loop() {
  int opcion;
  char texto[200];
  char robotId[] = "Daniel";
  DISPLAY_BT::print8("1=Activa Bluetooth", "2", "3", "4", "5", "6", "7", "8"); 
  delay(2000);
  opcion = botonDaniel.leeOpcion(8);
  sprintf(texto,"opcion=%d", opcion);  
  DISPLAY_BT::print4(texto, "oprima", "boton para", "continuar"); delay(1000);
  botonDaniel.esperaOprimir();
  delay(1000);
  switch (opcion){
    case 1: DISPLAY_BT::init_BT(robotId);  break;
    default: break;
  }
  DISPLAY_BT::printSerial(texto);   DISPLAY_BT::printSerial("\n");
  DISPLAY_BT::flushSerial();
  delay(1000);
}
