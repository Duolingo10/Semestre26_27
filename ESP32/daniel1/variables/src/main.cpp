#include<Arduino.h>

void setup() {
  Serial.begin(115200);
}
  
void loop(){
  int8_t  entero_con_signo = 0xFF;
  uint8_t entero_sin_signo = 0xFF;

  { bool v;        Serial.printf("%d byte(s) para el tipo: bool\n",sizeof(v)); }
  { char v;        Serial.printf("%d byte(s) para el tipo: char\n",sizeof(v)); }
  { byte v;        Serial.printf("%d byte(s) para el tipo: byte\n",sizeof(v)); }
  { int v;         Serial.printf("%d byte(s) para el tipo: int\n",sizeof(v)); }
  { long v;        Serial.printf("%d byte(s) para el tipo: long\n",sizeof(v)); }
  { long long v;   Serial.printf("%d byte(s) para el tipo: long long\n",sizeof(v)); }
  { float v;       Serial.printf("%d byte(s) para el tipo: float\n",sizeof(v)); }
  { double v;      Serial.printf("%d byte(s) para el tipo: double\n",sizeof(v)); }
  { long double v; Serial.printf("%d byte(s) para el tipo: long double\n",sizeof(v)); }
  Serial.printf("entero_con_signo con bits en 1s: %d\n", entero_con_signo);
  Serial.printf("entero_sin_signo con bits en 1s: %d\n", entero_sin_signo);
  delay(5000);
}