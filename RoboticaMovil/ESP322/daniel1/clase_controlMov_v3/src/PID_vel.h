#include<Arduino.h>
#define N_REG_ERR 300

class PID_vel{
 private:
   volatile static float Kp, Kd, Ki;     // Constantes del control PID
   volatile static int err0, sum_err, sum_err2; // errores
   volatile static int vel_max, vel_min, vel_op_der0;
   volatile static int acel_op_der;
 public:
   volatile static int  cnt;
   volatile static int  errores[N_REG_ERR];
   static void init(int v_min, int v_max, float kp, float kd, float ki);
   static int control(int vel_ref_izq, int pasos_izq, int pasos_der);
   static int get_sum_err2(void);
};  