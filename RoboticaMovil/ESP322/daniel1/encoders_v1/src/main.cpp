#include <Arduino.h>
#include <ESP32Encoder.h>	// Instalar la biblioteca ESP32Encoder
#include <macros.h>

ESP32Encoder encoder_izq, encoder_der;
volatile int g_pulsos_izq = 0, g_pulsos_der =0;

void setup() {
  Serial.begin(115200);
//  ESP32Encoder::useInternalWeakPullResistors=UP;
  encoder_izq.attachFullQuad(PIN_ENCB_IZQ, PIN_ENCA_IZQ);  // 28 pulsos por vuelta
  encoder_der.attachFullQuad(PIN_ENCB_DER, PIN_ENCA_DER);  // 28 pulsos por vuelta
//encoder_der.attachHalfQuad(PIN_ENCB_DER, PIN_ENCA_DER);  // 14 pulsos por vuelta
//encoder_der.attachSingleQuad(PIN_ENCB_DER, PIN_ENCA_DER);//  7 pulsos por vuelta
  encoder_izq.clearCount();   encoder_der.clearCount();    // fija contadores en 0
//encoder_izq.setCount(0);    encoder_der.setCount(0);
}

void loop(){
  g_pulsos_izq = encoder_izq.getCount();  g_pulsos_der = encoder_der.getCount();
  Serial.printf("izq = %4d  der = %4d \n", g_pulsos_izq, g_pulsos_der);
  delay(1000);
}