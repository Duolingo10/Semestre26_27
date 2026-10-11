#include <ESP32Encoder.h>
#include <Motores.h>
#include <Preferences.h>
#include <macros.h>

class ControlMov {
 private:
   static SemaphoreHandle_t mutex_tarea10ms;
   static Motores motoresDaniel;
   static ESP32Encoder encoderDanielIzq, encoderDanielDer;
   static int pasos_izq, pasos_izq0, pasos_der, pasos_der0; // contadores de pasos
   volatile static int vel_izq, vel_der;      // velocidades en pulsos/10ms
   static void tarea10ms(void *parameter);
   static uint8_t PWM_vel_izq[MAX_VEL], PWM_vel_der[MAX_VEL];
   volatile static int cnt_tarea10ms;    // contador de veces que corre tarea10ms
   volatile static bool controlPWM_activo;
   volatile static bool clear_encoders, ini_frena;
   volatile static int vel_op, vel_min, vel_max, vel_ref, mot_izq_giro, mot_der_giro;
   volatile static float acel_op_float;
   Preferences preferencias;
   void imprime_pwm_vel(char encabezado[], uint8_t pwm[], int n);
 public:
   static int getVelIzq(void);
   static int getVelDer(void);
   static int getPasosIzq(void);
   static int getPasosDer(void);
   bool initMotores(int PWMA, int AIN2, int AIN1, int STBY, int BIN1, int BIN2, int PWMB);
   bool initEncoders(int ENCB_IZQ, int ENCA_IZQ, int ENCB_DER, int ENCA_DER);
   void fijaPWMs(int pwmI, int pwmD);
   void activaMotores(void);
   void frenaMotores(void);
   void calibra_PWM_motores(void);
   void leeDatosFlash(void);
   static void avanza_vel(int vel=VEL_BAJA,int izq_giro=1,int der_giro=1,float aceleracion=ACEL_BAJA);
   static void avanza_vel(int vel=VEL_BAJA,float aceleracion=ACEL_BAJA);
   static int  avanza_pasos(int pasos, int vel=VEL_BAJA, int giro_izq=1, int giro_der=1, float aceleracion=ACEL_BAJA);
   static int  avanza_pasos(int pasos, int vel=VEL_BAJA, float aceleracion=ACEL_BAJA);
   static void para(void);
   static int  retrocede_pasos(int meta_pasos, int vel=VEL_BAJA, float aceleracion=ACEL_BAJA);
   static int  gira_pasos(int meta_pasos, int vel=VEL_BAJA, float aceleracion=ACEL_BAJA);
};
