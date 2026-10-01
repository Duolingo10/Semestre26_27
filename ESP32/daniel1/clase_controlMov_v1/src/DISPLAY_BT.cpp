#include <DISPLAY_BT.h>
#include <Preferences.h>
#include <macros.h>

BluetoothSerial DISPLAY_BT::SerialBT;
Adafruit_SSD1306 DISPLAY_BT::display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET,IIC_BPS_H,IIC_BPS_L);
bool DISPLAY_BT::SerialBT_listo = false;
bool DISPLAY_BT::SSD1306_listo = false;

bool DISPLAY_BT::init_SSD1306(void){
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C) == 0) { 
    SSD1306_listo = false;
    Serial.println(F("SSD1306 no se pudo inicializar"));
  } else {
    SSD1306_listo = true;
    display.clearDisplay();
    display.display();
  }
  return SSD1306_listo;
}

bool DISPLAY_BT::init_BT(char ID[]){
  SerialBT.begin(ID);
  Serial.printf("Esperando que la conexion BT se establezca en %s \n",ID); Serial.flush();
  print4("Esperando","conexion","por", "Bluetooth");
  while(SerialBT.connected() == false){
     Serial.print("*");  delay(500);
  }
  print4("Conexion","Bluetooth"," ","LISTA"); delay(2000);
  Serial.println("Conexion Bluetooth LISTA"); Serial.flush();
  SerialBT.println("Daniel 1");
  SerialBT_listo = true;
  return true;
}

void DISPLAY_BT::printSerial(const char texto[]){
  if(SerialBT_listo) {
    SerialBT.print(texto); SerialBT.flush();
  } else {
    Serial.print(texto); Serial.flush();
  }
}

float DISPLAY_BT::getFloatSerial(void){
  static float num;
  if(SerialBT_listo) {
    SerialBT.setTimeout(60000UL); // Espera hasta por 60s (default 1000ms)
    num = SerialBT.parseFloat();
    while(SerialBT.available()!=0) SerialBT.read();
  } else{
    Serial.setTimeout(60000UL); // Espera hasta por 60s (default 1000ms)
    num = Serial.parseFloat();
    while(Serial.available()!=0) Serial.read();
  }
  return num;
}

int DISPLAY_BT::getIntSerial(void){
  static int num;
  if(SerialBT_listo) {
    SerialBT.setTimeout(60000UL); // Espera hasta por 60s (default 1000ms)
    num = SerialBT.parseInt();
    while(SerialBT.available()!=0) SerialBT.read();
  } else{
    Serial.setTimeout(60000UL); // Espera hasta por 60s (default 1000ms)
    num = Serial.parseInt();
    while(Serial.available()!=0) Serial.read();
  }
  return num;
}

char DISPLAY_BT::getCharSerial(void){
  static char c;
  if(SerialBT_listo) {
    SerialBT.setTimeout(60000UL); // Espera hasta por 60s (default 1000ms)
    while(SerialBT.available()==0) delay(1);
    c = SerialBT.read();
  } else{
    Serial.setTimeout(60000UL); // Espera hasta por 60s (default 1000ms)
    while(Serial.available()==0) delay(1);
    c = Serial.read();
  }
  return c;
}

void DISPLAY_BT::getTextSerial(const char salida[], char texto[], int max_char){
  char c;
  int i = 0;

  if(SerialBT_listo) {
    SerialBT.print(salida);
    SerialBT.setTimeout(60000UL); // Espera hasta por 60s (default 1000ms)
    SerialBT.print(salida);
    while(SerialBT.available() == 0) delay(1);
    do{
      while(SerialBT.available() == 0) delay(1);
      c = SerialBT.read();
      if(c != '\n' && c != '\r') {
        texto[i] = c;
        i++;
      }
    } while (c != '\n' && c != '\r' && i < max_char-1);
    texto[i] = 0;
    if (SerialBT.available() != 0) SerialBT.read();
  } else {
    Serial.print(salida);
    Serial.setTimeout(60000UL); // Espera hasta por 60s (default 1000ms)
    Serial.print(salida);
    while(Serial.available() == 0) delay(1);
    do{
      while(Serial.available() == 0) delay(1);
      c = Serial.read();
      if(c != '\n' && c != '\r') {
        texto[i] = c;
        i++;
      }
    } while (c != '\n' && c != '\r' && i < max_char-1);
    texto[i] = 0;
    if (Serial.available() != 0) Serial.read();
  }
}

/* escala=1, despliega 4 lineas de texto de 10 caracteres
   escala=2, despliega 8 lineas de texto de 21 caracteres   */
void DISPLAY_BT::print(const char texto[], int escala){
  if(SSD1306_listo) {
    display.clearDisplay();
    display.setTextSize(escala);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.print(texto);
    display.display();
  }
}

void DISPLAY_BT::print(const char texto[],  const int escala, 
                       const char texto2[], const int escala2){
  if(SSD1306_listo) {
    display.clearDisplay();
    display.setTextSize(escala);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.print(texto);
    display.setTextSize(escala2);             // Normal 1:1 pixel scale
    if(texto2 != NULL) display.print(texto2);
    display.display();
  }
}

void DISPLAY_BT::print4(const char t1[], const  char t2[], 
                        const char t3[], const  char t4[]){
  if(SSD1306_listo) {
    display.clearDisplay();
    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println(t1); display.println(t2); display.println(t3);  display.println(t4);
    display.display();
  }
}

void DISPLAY_BT::print8(const char t1[],const char t2[],const char t3[],const char t4[], 
                        const char t5[],const char t6[],const char t7[],const char t8[]){
  if(SSD1306_listo) {
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println(t1); display.println(t2); display.println(t3);  display.println(t4);
    display.println(t5); display.println(t6); display.println(t7);  display.println(t8);
    display.display();
  }
}