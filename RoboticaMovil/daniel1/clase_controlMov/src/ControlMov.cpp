#include <Arduino.h>
#include <ControlMov.h>

#define TIMER0_10MS        10000  // 10 milisegundos

// Inicializacion de atributos estaticos de la clase
Motores ControlMov::motoresDaniel;
ESP32Encoder ControlMov::encoderDanielIzq; 
ESP32Encoder ControlMov::encoderDanielDer;
int ControlMov::pasos_izq=0, ControlMov::pasos_izq0=0;
int ControlMov::pasos_der=0, ControlMov::pasos_der0=0;
volatile int ControlMov::vel_izq=0,   ControlMov::vel_der=0;

// Se ejecuta en prioridad 2, nucleo 1, no requiere semaforo
void ControlMov::tarea10ms(void *parameter) {
 static TickType_t ticks0 = xTaskGetTickCount(), inc_ms = 10;
  while(true){
    xTaskDelayUntil(&ticks0, inc_ms);
    pasos_izq  = encoderDanielIzq.getCount();  pasos_der  = encoderDanielDer.getCount();
    vel_izq    = pasos_izq - pasos_izq0;       vel_der    = pasos_der - pasos_der0;
    pasos_izq0 = pasos_izq;                    pasos_der0 = pasos_der;
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

bool ControlMov::initEncoders(int ENCB_IZQ, int ENCA_IZQ, int ENCB_DER, int ENCA_DER){
  bool edo=false;
  encoderDanielIzq.attachFullQuad(ENCB_IZQ, ENCA_IZQ);  // 28 pulsos por vuelta
  encoderDanielDer.attachFullQuad(ENCB_DER, ENCA_DER);  // 28 pulsos por vuelta
  encoderDanielIzq.clearCount(); encoderDanielDer.clearCount();
  xTaskCreatePinnedToCore(tarea10ms,"tarea10ms",10000,NULL,2,NULL,1);
  return edo;
}

void ControlMov::fijaPWMs(int pwmI, int pwmD) { motoresDaniel.fijaPWMs(pwmI, pwmD); }
void ControlMov::activaMotores(void)          { motoresDaniel.activa(); }
void ControlMov::frenaMotores(void)           { motoresDaniel.frena(); }
