#include <Arduino.h>
#include <ControlMov.h>
#include <DISPLAY_BT.h>

#define TIMER0_10MS        10000  // 10 milisegundos

// Inicializacion de atributos estaticos de la clase
Motores ControlMov::motoresDaniel;
ESP32Encoder ControlMov::encoderDanielIzq; 
ESP32Encoder ControlMov::encoderDanielDer;
int ControlMov::pasos_izq=0, ControlMov::pasos_izq0=0;
int ControlMov::pasos_der=0, ControlMov::pasos_der0=0;
volatile int ControlMov::vel_izq=0,   ControlMov::vel_der=0;
volatile bool ControlMov::controlPWM_activo = false;
uint8_t  ControlMov::PWM_vel_izq[MAX_VEL]={0}, ControlMov::PWM_vel_der[MAX_VEL]={0};
int ControlMov::vel_max = 0;

// Se ejecuta en prioridad 2, nucleo 1, no requiere semaforo
void ControlMov::tarea10ms(void *parameter) {
 static TickType_t ticks0 = xTaskGetTickCount(), inc_ms = 10;
  while(true){
    xTaskDelayUntil(&ticks0, inc_ms);
    pasos_izq  = encoderDanielIzq.getCount();  pasos_der  = encoderDanielDer.getCount();
    vel_izq    = pasos_izq - pasos_izq0;       vel_der    = pasos_der - pasos_der0;
    pasos_izq0 = pasos_izq;                    pasos_der0 = pasos_der;
    if (controlPWM_activo){
      // Aqui va el control de los motores
    }
  }
}

int ControlMov::getVelIzq(void) {
  return vel_izq;
}

int ControlMov::getVelDer(void) {
  return vel_der;
}

bool ControlMov::initMotores(int PWMA, int AIN2, int AIN1, int STBY, int BIN1, int BIN2, int PWMB) {
    motoresDaniel.init(PWMA,AIN2,AIN1,STBY,BIN1,BIN2,PWMB);
  return true;
}

void ControlMov::leeDatosFlash(void){
  preferencias.begin("Daniel1", true);
  preferencias.getBytes("PWM_vel_izq",PWM_vel_izq,sizeof(PWM_vel_izq));
  preferencias.getBytes("PWM_vel_der",PWM_vel_der,sizeof(PWM_vel_der));
  vel_max = preferencias.getInt("vel_max",0);
  preferencias.end();
  Serial.println("Datos recuperados de memoria Flash:");
  { char encabezado_L[]= "PWM_vel_izq =", encabezado_R[]= "PWM_vel_der =";
    imprime_pwm_vel(encabezado_L, PWM_vel_izq, sizeof(PWM_vel_izq));
    imprime_pwm_vel(encabezado_R, PWM_vel_der, sizeof(PWM_vel_der)); }
  Serial.printf("vel_max=%d\n", vel_max);
}

void ControlMov::imprime_pwm_vel(char encabezado[], uint8_t pwm[], int n) {
  Serial.print(encabezado);
  for(int i=0; i < n; i++ )
    Serial.printf("(%d,%d)", i, pwm[i]);
  Serial.print("\n");
}


bool ControlMov::initEncoders(int ENCB_IZQ, int ENCA_IZQ, int ENCB_DER, int ENCA_DER){
  bool edo=false;
  encoderDanielIzq.attachFullQuad(ENCB_IZQ, ENCA_IZQ);  // 28 pulsos por vuelta
  encoderDanielDer.attachFullQuad(ENCB_DER, ENCA_DER);  // 28 pulsos por vuelta
  encoderDanielIzq.clearCount(); encoderDanielDer.clearCount();
  xTaskCreatePinnedToCore(tarea10ms,"tarea10ms",10000,NULL,2,NULL,1);
  leeDatosFlash();
  return edo;
}

void ControlMov::fijaPWMs(int pwmI, int pwmD) { motoresDaniel.fijaPWMs(pwmI, pwmD); }
void ControlMov::activaMotores(void)          { motoresDaniel.activa(); }
void ControlMov::frenaMotores(void)           { motoresDaniel.frena(); }

void ControlMov::calibra_PWM_motores(void){ 
  int maxPWM = 255, pwm, i, m, v, v0, v_izq_max, v_der_max;
  char texto[80], texto1[80], texto2[80];
  controlPWM_activo = false;
  for(v=0; v < MAX_VEL; v++) PWM_vel_izq[v]=PWM_vel_der[v]=0;
  for (m = 0; m < 2; m++){
    v0 = 1;
    for(pwm = 1; pwm <= maxPWM; pwm++){  // acelera lentamente motor
      (m==0) ? fijaPWMs(pwm,0): fijaPWMs(0,pwm);
      delay(300);
      v = (m==0)? getVelIzq(): getVelDer();
      sprintf(texto,"Mot = %d",m);
      sprintf(texto1,"pwm = %3d",pwm);
      sprintf(texto2,"vel = %3d",v);
      DISPLAY_BT::print4("CalibraPWM",texto,texto1,texto2);
      if(v > v0) {
        for(i = v0; i <= v; i++) 
          if (i < MAX_VEL) 
            if(m==0) PWM_vel_izq[i] = pwm;
            else     PWM_vel_der[i] = pwm;
        v0 = v;
      }
    }
    if (m==0) v_izq_max = v;
    else      v_der_max = v;
    if (m==0) fijaPWMs(pwm/2,    0); 
    else      fijaPWMs(0,    pwm/2);
    delay(2000);
    fijaPWMs(0,0); delay(2000);
  }
  if(v_izq_max < v_der_max) 
    vel_max = v_izq_max;
  else
    vel_max = v_der_max;

  preferencias.begin("Daniel1", false);
  preferencias.putBytes("PWM_vel_izq",PWM_vel_izq,sizeof(PWM_vel_izq));
  preferencias.putBytes("PWM_vel_der",PWM_vel_der,sizeof(PWM_vel_der));
  preferencias.putInt("vel_max",vel_max);
  preferencias.end();
}
