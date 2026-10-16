#include <Arduino.h>
#include <ControlMov.h>
#include <DISPLAY_BT.h>

#define TIMER0_10MS        10000  // 10 milisegundos

// Inicializacion de atributos estaticos de la clase
volatile int  ControlMov::cnt_tarea10ms= 0;
SemaphoreHandle_t ControlMov::mutex_tarea10ms;
Motores ControlMov::motoresDaniel;
ESP32Encoder ControlMov::encoderDanielIzq; 
ESP32Encoder ControlMov::encoderDanielDer;
int ControlMov::pasos_izq=0, ControlMov::pasos_izq0=0;
int ControlMov::pasos_der=0, ControlMov::pasos_der0=0;
volatile int   ControlMov::vel_izq=0,   ControlMov::vel_der=0;
volatile int   ControlMov::vel_min = 7, ControlMov::vel_max = 10;
volatile int   ControlMov::vel_op =0,   ControlMov::vel_ref=0;
volatile float ControlMov::acel_op_float=1.0;  //  valores solo positivos
volatile int   ControlMov::mot_izq_giro = 1, ControlMov::mot_der_giro = 1;
volatile bool  ControlMov::controlPWM_activo = false;
volatile bool  ControlMov::clear_encoders = true, ControlMov::ini_frena = false; 
uint8_t  ControlMov::PWM_vel_izq[MAX_VEL]={0}, ControlMov::PWM_vel_der[MAX_VEL]={0};

// *** Esta tarea se ejecuta con prioridad 2, en nucleo 1 ***
void ControlMov::tarea10ms(void *parameter) {
  static TickType_t ticks0 = xTaskGetTickCount(), inc_ms = 10;
  static int   PWM_izq=0, PWM_der=0;
  static float vel_op_float = 0;
  while(true){
    xTaskDelayUntil(&ticks0, inc_ms);
    xSemaphoreTake(mutex_tarea10ms, portMAX_DELAY);
    if(clear_encoders){ // Inicializa valores de las variables
      pasos_izq0=pasos_der0=vel_op=cnt_tarea10ms=0; vel_op_float = 0.0;
      encoderDanielIzq.clearCount(); encoderDanielDer.clearCount();
      clear_encoders = false;
    }
    pasos_izq=encoderDanielIzq.getCount()*mot_izq_giro; // pasos_izq son positivos
    vel_izq=pasos_izq-pasos_izq0; pasos_izq0=pasos_izq; 
    pasos_der=encoderDanielDer.getCount()*mot_der_giro; // pasos_der son positivos
    vel_der=pasos_der-pasos_der0; pasos_der0=pasos_der; 
    if (controlPWM_activo){
      if(!ini_frena && vel_op_float < (float)vel_ref)//Arranque 
        vel_op_float += acel_op_float;                     // sube velocidad
      if(ini_frena) vel_op_float -= acel_op_float;         // baja velocidad
      if(vel_op_float > (float) vel_ref) vel_op_float = vel_ref;
      if(vel_op_float < (float) vel_min) vel_op_float = (float) vel_min;
      vel_op = (int) vel_op_float;
      PWM_izq = PWM_vel_izq[vel_op]; PWM_der = PWM_vel_der[vel_op];
      motoresDaniel.fijaPWMs(PWM_izq*mot_izq_giro, PWM_der*mot_der_giro);
    }
    cnt_tarea10ms++;
    xSemaphoreGive(mutex_tarea10ms);
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
  mutex_tarea10ms = xSemaphoreCreateMutex();
  if(mutex_tarea10ms == NULL) { Serial.println("*** Falla en mutex_tarea10ms ***"); }
  xSemaphoreGive(mutex_tarea10ms);
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

/* Fija ambas ruedas a velocidad vel (positiva) con aceleracion (positiva)
   giro=1 se mueve hacia adeltante; giro=-1 se mueve hacia atras  */
void ControlMov::avanza_vel(int vel, int izq_giro, int der_giro, float aceleracion) {
  int c;
  if(controlPWM_activo) 
    return;
  xSemaphoreTake(mutex_tarea10ms, portMAX_DELAY);
  c = cnt_tarea10ms;
  clear_encoders = true;
  if(vel <  vel_min) vel = vel_min;
  if(vel >= vel_max) vel = vel_max;
  vel_ref = vel;
  acel_op_float = aceleracion;
  mot_izq_giro = izq_giro; mot_der_giro = der_giro;
  ini_frena = false;
  controlPWM_activo = true;
  xSemaphoreGive(mutex_tarea10ms);
  while(c == cnt_tarea10ms)  delay(1);  // espera siguiente ejecucion de tarea10ms
}

// Avanza ambas ruedas a velocidad vel, considerando aceleracion
void ControlMov::avanza_vel(int vel, float aceleracion) {
   Serial.printf("vel=%d\n",vel);
   avanza_vel(vel, 1, 1, aceleracion);
}

void ControlMov::para(void) {
  xSemaphoreTake(mutex_tarea10ms, portMAX_DELAY);
  controlPWM_activo = false;
  xSemaphoreGive(mutex_tarea10ms);
  motoresDaniel.frena();
}

// avanza meta_pasos (positivo), considerando los giros de los motores
int ControlMov::avanza_pasos(int meta_pasos,int vel,int giro_izq,int giro_der,float acel){
  int pasos_para_frenar, p_izq, p_der;
  char texto[100];
  if(controlPWM_activo==false) avanza_vel(vel,giro_izq,giro_der,acel); //robot parado
  else meta_pasos += getPasosIzq();                   // continuacion del movimiento
  pasos_para_frenar = meta_pasos - (int)((vel_op/2.0)*(vel_op/acel)); // vel_media * tiempo
  while (getPasosIzq() < pasos_para_frenar) delay(1); // Espera para frenar
  ini_frena =true;                                    // inicia frenado
  while(getPasosIzq() < meta_pasos) delay(1);//espera que motor izq alcance meta_pasos
  para();
  return getPasosIzq();
}

int ControlMov::avanza_pasos(int meta_pasos, int vel, float aceleracion) {
  return avanza_pasos(meta_pasos, vel, 1, 1, aceleracion);
}

int ControlMov::getPasosIzq(void) {
  static int pasos=0;
  xSemaphoreTake(mutex_tarea10ms, portMAX_DELAY);
  pasos = encoderDanielIzq.getCount() * mot_izq_giro;
  xSemaphoreGive(mutex_tarea10ms);
  return pasos;
}

int ControlMov::getPasosDer(void) {
  static int pasos=0;
  xSemaphoreTake(mutex_tarea10ms, portMAX_DELAY);
  pasos = encoderDanielDer.getCount() * mot_der_giro;
  xSemaphoreGive(mutex_tarea10ms);
  return pasos;
}

int ControlMov::retrocede_pasos(int meta_pasos, int vel, float aceleracion) {
  return avanza_pasos(meta_pasos, vel, -1, -1, aceleracion);
}

int ControlMov::gira_pasos(int meta_pasos, int vel, float aceleracion) {
  int pasos;
  if(meta_pasos > 0)  // giro en sentido contrario a las manecillas del reloj
    pasos=avanza_pasos(meta_pasos,vel,-1,1,aceleracion); //m_izq atras m_der avanza
  else                // giro en sentido de las manecillas del reloj
    pasos=-1*avanza_pasos(-meta_pasos,vel,1,-1,aceleracion);//m_izq avanza m_der atras
  return pasos;
}
