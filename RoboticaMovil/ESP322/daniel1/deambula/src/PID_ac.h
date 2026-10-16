#include<Arduino.h>
#define N_REG_VEL 300

class PID_ac{
 private:
   volatile static float Kp_ap, Kd_ap, Ki_ap;     // Constantes PID aceleracion
   volatile static float Kp_an, Kd_an, Ki_an;     // Constantes PID frenado
   volatile static int vel_max, vel_min, vel0;
   volatile static float vel_op_float;
   volatile static int err0, sum_err, sum_err2_ap, sum_err2_an; // errores
 public:
   volatile static byte velOp[N_REG_VEL];
   volatile static byte velIzq[N_REG_VEL];
   volatile static byte velDer[N_REG_VEL];
   volatile static int  cnt;
   static void init(int v_min, int v_max);
   static int control(int vel_ref_izq, int vel_izq, int vel_der, bool frenado, float acel_op_float);
   static int get_sum_err2_ap(void);
   static int get_sum_err2_an(void);
};  