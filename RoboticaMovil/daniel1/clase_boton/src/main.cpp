#include <Arduino.h>
#include <Boton.h>
#ifdef DEBUG_BLUETOOTH
#include <BluetoothSerial.h>
BluetoothSerial SerialBT;
void initBT(void) {
  SerialBT.begin("ESP32-Daniel");
  Serial.print("Esperando que la conexion BT se establezca: "); Serial.flush();
  while(SerialBT.connected() == false){
     Serial.print(".");
     delay(500);
  }
  Serial.println(" LISTA"); Serial.flush();
}
#endif

Boton botonDaniel;

void setup() {
  pinMode(PIN_STBY,OUTPUT); digitalWrite(PIN_STBY,LOW);
  Serial.begin(115200);
  delay(3000); 
#ifdef DEBUG_BLUETOOTH
  initBT();
#endif
  botonDaniel.init(PIN_BOTON, PIN_LED);
  Serial.println("Robot Daniel ... listo");  Serial.flush();
#ifdef DEBUG_BLUETOOTH
  SerialBT.println("Robot Daniel ... listo");  SerialBT.flush();
#endif
}

void loop() {
  static int opcion=0;
  Serial.printf("Inicio de lectura del boton\n"); Serial.flush();
  opcion = botonDaniel.leeOpcion(4);
  Serial.printf("opcion = %d\n", opcion); Serial.flush();
#ifdef DEBUG_BLUETOOTH
  SerialBT.printf("opcion = %d\n", opcion); SerialBT.flush();
#endif
  Serial.printf("Espera a que se oprima el boton\n"); Serial.flush();
  botonDaniel.esperaOprimir();
  switch (opcion){
    case 3:  botonDaniel.bateria();       break;
    case 4:  botonDaniel.calibra_bat();   break;
    default:                              break;
  }
  delay(5000);
}
