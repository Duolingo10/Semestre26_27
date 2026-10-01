#ifndef MACROS_DANIEL
 #include<macros.h>
#endif

class SiguePared{
  private:
  static int mm[VL6180X_N];
  static float mmF[VL6180X_N], mmProm[VL6180X_N];
  static ulong tiempo_ms; 
  static char texto[80];
  public:
    static void sigueParedIzq(void);
};