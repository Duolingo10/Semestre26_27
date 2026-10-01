#include<PID_ac.h>

volatile float PID_ac::Kp_ap=1.0, PID_ac::Kd_ap=0.0f, PID_ac::Ki_ap=0.0f; //acel positiva  
volatile float PID_ac::Kp_an=5, PID_ac::Kd_an=0.0f, PID_ac::Ki_an=0.5f; //acel negativa
volatile int   PID_ac::vel_min=0, PID_ac::vel_max=0, PID_ac::vel0=0;
volatile float PID_ac::vel_op_float=0.0f;
volatile int   PID_ac::err0=0,PID_ac::sum_err=0, PID_ac::sum_err2_ap, PID_ac::sum_err2_an; // errores
volatile byte  PID_ac::velOp[N_REG_VEL]={0},PID_ac::velIzq[N_REG_VEL]={0},PID_ac::velDer[N_REG_VEL] ={0};
volatile int   PID_ac::cnt=0;

void PID_ac::init(int v_min, int v_max){
  vel_min = v_min; vel_max = v_max;
  err0 = sum_err = sum_err2_ap = sum_err2_an = cnt = 0;
  vel_op_float = (float) vel_min;
}

int PID_ac::control(int vel_ref, int vel_izq, int vel_der, bool frenado, float acel_op_float){
  static int err, vel_op, control, vel_control;
  if(!frenado) {
    if(!frenado && vel_op_float < (float)vel_ref)//Arranque 
      vel_op_float += acel_op_float;                     // sube velocidad
      vel_op = (int) vel_op_float;
      err = vel_op - vel_izq;
      control = Kp_ap * err + Kd_ap*(err - err0) + Ki_ap*sum_err;
      err0 = err; sum_err += err;
      sum_err2_ap += err*err;
  }
  else {
    vel_op = vel_min;
    err = vel_op - vel_izq;
    control = Kp_an * err + Kd_an*(err - err0) + Ki_an*sum_err;
    err0 = err; sum_err += err;
    sum_err2_an += err*err;
  }
  err0 = err;
  sum_err += err;
  vel_control = vel_op + control;
  if(vel_control > vel_ref) vel_control = vel_ref;
  if(vel_control < vel_min) vel_control = vel_min;
  if (cnt < N_REG_VEL) {velOp[cnt]=vel_control; velIzq[cnt] = vel_izq; velDer[cnt] = vel_der; cnt++; }
  return vel_control;
}


