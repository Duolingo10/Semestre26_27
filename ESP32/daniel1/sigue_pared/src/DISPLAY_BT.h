#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BluetoothSerial.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define BUFFER_MAX_Ren 100
#define BUFFER_MAX_Col 100


class DISPLAY_BT {
 private:
   static bool SSD1306_listo; 
   static bool SerialBT_listo;
   static Adafruit_SSD1306 display;
   static BluetoothSerial SerialBT;
   static char buffer[BUFFER_MAX_Ren][BUFFER_MAX_Col];
   static int buffer_r_free;
 public:
   static bool  init_SSD1306(void);
   static bool  init_BT(char ID[]);
   static void  printSerial(const char texto[]);
   static void  flushSerial(void);
   static void  clearSerial(void);
   static float getFloatSerial(void);
   static int   getIntSerial(void);
   static char  getCharSerial(void);
   static void  getTextSerial(const char salida[], char texto[], int max_char);
   static void  print(const char texto[], int escala);
   static void  print(const char texto[], int escala, const char texto2[], int escala2);
   static void  print4(const char t1[], const char t2[], const char t3[], const char t4[]);
   static void  print8(const char t1[], const char t2[], const char t3[], const char t4[], const char t5[], const char t6[], const char t7[], const char t8[]);
};