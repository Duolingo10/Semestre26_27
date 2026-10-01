#include <ESP32Encoder.h>
#include <Motores.h>

class ControlMov {
 private:
   static Motores motoresDaniel;
   static ESP32Encoder encoderDanielIzq, encoderDanielDer;
   static int pasos_izq, pasos_izq0, pasos_der, pasos_der0; // contadores de pasos
   volatile static int vel_izq, vel_der;      // velocidades en pulsos/10ms
   static void tarea10ms(void *parameter);
 public:
   static int getVelIzq(void);
   static int getVelDer(void);
   bool initMotores(int PWMA, int AIN2, int AIN1, int STBY, int BIN1, int BIN2, int PWMB);
   bool initEncoders(int ENCB_IZQ, int ENCA_IZQ, int ENCB_DER, int ENCA_DER);
   void fijaPWMs(int pwmI, int pwmD);
   void activaMotores(void);
   void frenaMotores(void);
};
