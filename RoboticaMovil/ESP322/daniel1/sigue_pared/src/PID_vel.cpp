#include<PID_vel.h>

volatile float PID_vel::Kp = 0.25, PID_vel::Kd = 0.75, PID_vel::Ki= 0.0;
volatile int   PID_vel::err0=0;
volatile int   PID_vel::sum_err=0,PID_vel::sum_err2=0;
volatile int   PID_vel::vel_max=10, PID_vel::vel_min=1;
volatile int   PID_vel::vel_op_der0, PID_vel::acel_op_der= 20;
volatile int   PID_vel::errores[N_REG_ERR]={0};
volatile int   PID_vel::cnt=0;


void PID_vel::init(int v_min, int v_max, float kp, float kd, float ki){
  vel_min = v_min; vel_max = v_max;
  Kp = kp; Kd = kd; Ki = ki;
  err0 = sum_err = sum_err2 = cnt = 0;
  vel_op_der0 = -1;
}

int PID_vel::control(int vel_ref_izq, int pasos_izq, int pasos_der){
  static int control, err, vel_op_der;
  err = pasos_izq - pasos_der; 
  sum_err += err;
  control = Kp*err + Kd*(err-err0)+Ki*sum_err;
  err0 = err;
  sum_err2 += err * err;
  vel_op_der = vel_ref_izq + control;
  if (vel_op_der0 == -1) vel_op_der0 = vel_ref_izq;
  if (vel_op_der > vel_op_der0 + acel_op_der) vel_op_der = vel_op_der0 + acel_op_der;
  if (vel_op_der < vel_op_der0 - acel_op_der) vel_op_der = vel_op_der0 - acel_op_der;
  if (vel_op_der >= vel_max) vel_op_der = vel_max;
  if (vel_op_der < 0)  vel_op_der = 0;
  vel_op_der0 = vel_op_der;
  if (cnt < N_REG_ERR) {errores[cnt] = err; cnt++;}
  return vel_op_der;
}

int PID_vel::get_sum_err2(void){
    return sum_err2;
}