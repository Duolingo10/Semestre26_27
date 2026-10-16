#include <Arduino.h>
#include <ControlMov.h>
#include <DISPLAY_BT.h>
#include <PID_vel.h>
#include <PID_ac.h>
#include <macros.h>

#define TIMER0_10MS        10000  // 10 milisegundos

// Inicializacion de atributos estaticos de la clase
volatile float ControlMov::FC_grados_pasos = GRADOS_A_PASOS(1);
char          ControlMov::robot_ID[] ="Daniel1";
volatile int  ControlMov::cnt_tarea10ms= 0;
SemaphoreHandle_t ControlMov::mutex_tarea10ms;
Motores ControlMov::motoresDaniel;
ESP32Encoder ControlMov::encoderDanielIzq; 
ESP32Encoder ControlMov::encoderDanielDer;
int ControlMov::pasos_izq=0, ControlMov::pasos_izq0=0;
int ControlMov::pasos_der=0, ControlMov::pasos_der0=0;
volatile int   ControlMov::vel_izq=0,   ControlMov::vel_der=0;
volatile int   ControlMov::vel_min = MIN_VEL, ControlMov::vel_max = 10;
volatile int   ControlMov::pasos_frenado=0, ControlMov::pasos_frenado2=0;
volatile int   ControlMov::vel_ref=0;
volatile float ControlMov::Kp=0.25f, ControlMov::Kd=0.75f, ControlMov::Ki=0.0f;
volatile int   ControlMov::e0 = 0, ControlMov::e_min =0;
volatile float ControlMov::acel_op_float=1.0;  //  valores solo positivos
volatile int   ControlMov::mot_izq_giro = 1, ControlMov::mot_der_giro = 1;
volatile bool  ControlMov::controlPWM_activo = false;
volatile bool  ControlMov::clear_encoders = true, ControlMov::ini_frena = false; 
uint8_t  ControlMov::PWM_vel_izq[MAX_VEL]={0}, ControlMov::PWM_vel_der[MAX_VEL]={0};
volatile int   ControlMov::calibra_perturbacion = 0, ControlMov::offsetEncDer=0;

// *** Esta tarea se ejecuta con prioridad 2, en nucleo 1 ***
void ControlMov::tarea10ms(void *parameter) {
  static TickType_t ticks0 = xTaskGetTickCount(), inc_ms = 10;
  static int PWM_izq=0, PWM_der=0, pasos, vel_op_izq, vel_op_der;
  while(true){
    xTaskDelayUntil(&ticks0, inc_ms);
    xSemaphoreTake(mutex_tarea10ms, portMAX_DELAY);
    if(clear_encoders){ // Inicializa valores de las variables
      pasos_izq0=pasos_der0=cnt_tarea10ms=0;
      encoderDanielIzq.clearCount(); encoderDanielDer.clearCount();
      PID_vel::init(vel_min,vel_max,Kp,Kd,Ki);  // Inicializa Control PID_vel
      PID_ac::init(vel_min,vel_max);            // Inicializa Control PID_ac
      clear_encoders = false;
    }
    pasos_izq=encoderDanielIzq.getCount()*mot_izq_giro; // pasos_izq son positivos
    vel_izq=pasos_izq - pasos_izq0; pasos_izq0 = pasos_izq; 
    pasos_der=encoderDanielDer.getCount()*mot_der_giro; // pasos_der son positivos
    vel_der=pasos_der - pasos_der0; pasos_der0 = pasos_der; 

    if((calibra_perturbacion>0) && (cnt_tarea10ms==calibra_perturbacion))
      offsetEncDer = GRADOS_A_PASOS(15)*2;
    if(offsetEncDer != 0) {
      pasos = encoderDanielDer.getCount() + offsetEncDer;
      encoderDanielDer.setCount(pasos);
      offsetEncDer = 0;
    }

    if (controlPWM_activo){
      vel_op_izq = PID_ac::control(vel_ref, vel_izq, vel_der, ini_frena, acel_op_float);
      vel_op_der = PID_vel::control(vel_op_izq, pasos_izq, pasos_der); 

      PWM_izq = PWM_vel_izq[vel_op_izq]; PWM_der = PWM_vel_der[vel_op_der];
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
  String s;
  preferencias.begin("Daniel1", true);
  s = preferencias.getString("robot_ID", "Daniel1");
  strcpy(robot_ID, s.c_str());
  preferencias.getBytes("PWM_vel_izq",PWM_vel_izq,sizeof(PWM_vel_izq));
  preferencias.getBytes("PWM_vel_der",PWM_vel_der,sizeof(PWM_vel_der));
  vel_max = preferencias.getInt("vel_max",0);
  Kp = preferencias.getFloat("Kp",0.0);
  Kd = preferencias.getFloat("Kd",0.0);
  Ki = preferencias.getFloat("Ki",0.0);
  e0 = preferencias.getInt("e0", 0);
  e_min=preferencias.getInt("e_min", 0);
  FC_grados_pasos = preferencias.getFloat("FC_grados_pasos",FC_grados_pasos);
  pasos_frenado = preferencias.getInt("pasos_frenado", 0);
  pasos_frenado2 = preferencias.getInt("pasos_frenado2", 0);
  preferencias.end();
  Serial.println("Datos recuperados de memoria Flash:");
  Serial.printf("robot_ID=%s\n", robot_ID);
  { char encabezado_L[]= "PWM_vel_izq =", encabezado_R[]= "PWM_vel_der =";
    imprime_pwm_vel(encabezado_L, PWM_vel_izq, sizeof(PWM_vel_izq));
    imprime_pwm_vel(encabezado_R, PWM_vel_der, sizeof(PWM_vel_der)); }
  Serial.printf("vel_max=%d\n", vel_max);
  Serial.printf("Kp=%f Kd=%f Ki=%f e0=%d e_min=%d\n",Kp, Kd, Ki,e0,e_min);
  Serial.printf( "pasos_frenado=%d pasos_frenado2=%d\n",pasos_frenado,pasos_frenado2);
  Serial.printf("FC_grados_pasos = %f\n", FC_grados_pasos);
}

void ControlMov::set_robot_ID(void){
  char texto[80];
  Preferences preferencias;
  DISPLAY_BT::getTextSerial("Ingrese el identificador del robot (max 20 caracteres) y termine el nombre con enter\nID:", robot_ID, 20);
  sprintf(texto,"\nIngreso ID: %s\n",robot_ID);
  DISPLAY_BT::printSerial(texto); DISPLAY_BT::flushSerial();
  preferencias.begin("Daniel1", false);
  preferencias.putString("robot_ID", String(robot_ID));
  preferencias.end();
}

void ControlMov::get_robot_ID(char id[]){
  strcpy(id, robot_ID);
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
  leeDatosFlash();
  xTaskCreatePinnedToCore(tarea10ms,"tarea10ms",10000,NULL,2,NULL,1);
  return edo;
}

void ControlMov::fijaPWMs(int pwmI, int pwmD) { motoresDaniel.fijaPWMs(pwmI, pwmD); }
void ControlMov::activaMotores(void)          { motoresDaniel.activa(); }
void ControlMov::frenaMotores(void)           { motoresDaniel.frena(); }

void ControlMov::calibra_PWM_motores(Boton boton){ 
  int maxPWM = 255, pwm, i, m, v, v0, v_izq_max, v_der_max;
  char texto[80], texto1[80], texto2[80];
  controlPWM_activo = false;
  for(v=0; v < MAX_VEL; v++) PWM_vel_izq[v]=PWM_vel_der[v]=0;
  for (m = 0; m < 2; m++){
    v0 = 1;
    for(pwm = 1; pwm <= maxPWM; pwm++){  // acelera lentamente motor
      (m==0) ? fijaPWMs(pwm,0): fijaPWMs(0,pwm);
      delay(500);
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

  sprintf(texto, "FIN\nCalibra\nvmax=%3d\n%3d %3d\n",vel_max, v_izq_max,v_der_max);
  DISPLAY_BT::print(texto,2);
  boton.esperaOprimir();
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
   avanza_vel(vel, 1, 1, aceleracion);
}

void ControlMov::para(void) {
  int c;
  xSemaphoreTake(mutex_tarea10ms, portMAX_DELAY);
  c = cnt_tarea10ms;
  controlPWM_activo = false;
  xSemaphoreGive(mutex_tarea10ms);
  motoresDaniel.frena();
  while(c == cnt_tarea10ms)  delay(1);  // espera siguiente ejecucion de tarea10ms
}

// avanza meta_pasos (positivo), considerando los giros de los motores
int ControlMov::avanza_pasos(int meta_pasos,int vel,int giro_izq,int giro_der,float acel){
  int pasos_para_frenar, p_izq, p_der, p_izq0, p_der0, dif_izq, dif_der, pasos_etapa0, sum_err2_etapa0;
  char texto[100];
  if(controlPWM_activo == false) avanza_vel(vel,giro_izq,giro_der,acel); //robot parado
  else meta_pasos += getPasosIzq();                   // continuacion del movimiento
  pasos_para_frenar = meta_pasos - pasos_frenado;     // calcula pasos para iniciar frenado
  while (getPasosIzq() < pasos_para_frenar) delay(1); // Espera para frenar
  ini_frena =true;                                    // inicia frenado
  while(getVelIzq() > vel_min+1)    delay(1);         // espera a que baje la velocidad
  pasos_para_frenar = meta_pasos - pasos_frenado2;    // calcula pasos para detener el robot
  while(getPasosIzq() < pasos_para_frenar) delay(1);
  para();
  while( getVelIzq() > 0 || getVelDer() > 0 ) delay(1);
/*  for(int i=0; i < PID_ac::cnt; i++) {
    sprintf(texto,"i=%3d vel_ref=%3d vel_izq=%3d vel_der=%3d, err=%d\n", 
      i, PID_ac::velOp[i],PID_ac::velIzq[i],PID_ac::velDer[i], PID_vel::errores[i]);
    DISPLAY_BT::printSerial(texto); DISPLAY_BT::flushSerial();
  }  */
  sprintf(texto,"meta_pasos=%d pasos_izq=%d pasos_der=%d pasos_frenado=%d pasos_frenado2=%d\n", 
    meta_pasos, getPasosIzq(), getPasosDer(), pasos_frenado, pasos_frenado2);
  DISPLAY_BT::printSerial(texto); DISPLAY_BT::flushSerial();  
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
  if (meta_pasos == 0) return 0;
  if (meta_pasos > 0)  // giro en sentido contrario a las manecillas del reloj
    pasos = avanza_pasos(meta_pasos,vel,-1,1,aceleracion); //mot_izq atras mot_der avanza
  else                // giro en sentido de las manecillas del reloj
    pasos = -1*avanza_pasos(-meta_pasos,vel,1,-1,aceleracion);//mot_izq avanza mot_der atras
  return pasos;
}

int ControlMov::conv_grados_a_pasos(float grados){
  int pasos;
  pasos = grados * FC_grados_pasos;
  return pasos;
}

int ControlMov::gira_grados(float grados, int vel, float aceleracion){
  int pasos = conv_grados_a_pasos(grados);
  return gira_pasos(pasos, vel, aceleracion);
}

void ControlMov::SetOffsetEncD(int val){
  xSemaphoreTake(mutex_tarea10ms, portMAX_DELAY);
  offsetEncDer = val;
  xSemaphoreGive(mutex_tarea10ms);
}

void ControlMov::setKpKdKi(float kp, float kd, float ki) {
  xSemaphoreTake(mutex_tarea10ms, portMAX_DELAY);
  Kp = kp; Kd = kd; Ki = ki;
  xSemaphoreGive(mutex_tarea10ms);
}

void ControlMov::estimaKpKdKi(void) {
  Preferences preferencias;
  int e0, e=0, e_min=-1,  i, j, k, p, n = 3, m=5, it=0, mm=200;
  float Kp=0,Kp0=0,Kp_min=0,Kd=0,Kd0=0,Kd_min=0,Ki=0,Ki0=0,Ki_min=0,
        incKp=1.0f, incKd=1.0f, incKi=0.001f;
  bool adelante = true;
  char texto[120];
  calibra_perturbacion = 20;
  for(p=0; p < m; p++){
    for(i=0; i<n; i++) {
      for(j=0; j<n; j++) {
        for(k=0; k<1; k++) {
          Kp = Kp0+i*incKp; Kd=Kd0+j*incKd; Ki=Ki0+k*incKi;
          setKpKdKi(Kp, Kd, Ki);
          sprintf(texto, "it = %d\n",it); 
          DISPLAY_BT::print(texto,2);
          if(adelante) 
            avanza_pasos(MM_A_PASOS(mm),20,1.0f);
          else 
            retrocede_pasos(MM_A_PASOS(mm),20,1.0f); 
          e = PID_vel::get_sum_err2();
          sprintf(texto, "it=%d Kp=%f Kd=%f Ki=%f e=%d\n",it,Kp,Kd,Ki,e); 
          DISPLAY_BT::printSerial(texto);
          sprintf(texto, "Kp=%4.2f\nKd=%4.2f\nKi%.4f\ne=%d\n",Kp,Kd,Ki,e); 
          DISPLAY_BT::print(texto,2);
          if(adelante) delay(2000); else delay(3000);
          adelante = !adelante;
          if(e_min == -1 || e < e_min) {Kp_min=Kp; Kd_min=Kd; Ki_min=Ki, e_min=e;}
          if(Kp==0 && Kd ==0 && Ki == 0) e0 = e;
          it++;
        }
      }
    }
    sprintf(texto,"Kp_min=%f Kd_min=%f Ki_min=%f e0=%d e_min=%d\n",Kp_min,Kd_min,Ki_min,e0,e_min); 
    DISPLAY_BT::printSerial(texto);
    sprintf(texto, "Kp*=%4.2f\nKd*=%4.2f\nKi*=%4.2f\ne*=%d\n",Kp_min,Kd_min,Ki_min,e_min); 
    DISPLAY_BT::print(texto,2);
    delay(5000);
    incKp /=2.0f; incKd /=2.0f; incKi /=2.0f;
    Kp0 = Kp_min -incKp; Kd0 = Kd_min -incKd; Ki0 = Ki_min - incKi; 
    if(Kp0 < 0) Kp0 = 0;   if(Kd0 < 0) Kd0 = 0;   if(Ki0 < 0) Ki0 = 0;
  }
  setKpKdKi(Kp_min, Kd_min, Ki_min);
  preferencias.begin("Daniel1", false);
  preferencias.putFloat("Kp",Kp_min);
  preferencias.putFloat("Kd",Kd_min);
  preferencias.putFloat("Ki",Ki_min);
  preferencias.putInt("e0", e0);
  preferencias.putInt("e_min", e_min);
  preferencias.end();
  calibra_perturbacion = 0;
  calibra_frenado();
}

void ControlMov::calibra_frenado(void){
  Preferences preferencias;
  int dist_mm = 100, pasos_recorridos, meta_pasos;
  char texto[100];
  meta_pasos = MM_A_PASOS(dist_mm);
  pasos_frenado = pasos_frenado2 = 0;
  pasos_recorridos = avanza_pasos(meta_pasos, VEL_BAJA, ACEL_BAJA); delay(1000);
  pasos_frenado = pasos_recorridos - meta_pasos;  // fija parametro global
  pasos_recorridos = retrocede_pasos(meta_pasos, VEL_BAJA, ACEL_BAJA); delay(1000);
  pasos_frenado2 = pasos_recorridos - meta_pasos;
  pasos_recorridos = avanza_pasos(meta_pasos, VEL_BAJA, ACEL_BAJA); delay(1000);
  sprintf(texto, "avanza %d\n real %d\np_f=%d\np_f2=%d",meta_pasos, pasos_recorridos, pasos_frenado, pasos_frenado2);
  DISPLAY_BT::print(texto,2); delay(10000);
  preferencias.begin("Daniel1", false);
  preferencias.putInt("pasos_frenado",pasos_frenado);
  preferencias.putInt("pasos_frenado2",pasos_frenado2);
  preferencias.end();
}

void ControlMov::calibra_giro(void){
  Preferences preferencias;

  char texto[255], c;
  bool salir = false;
  int pasos;
  sprintf(texto,"Se calcula que alrededor de %d pasos se necesitan para una vuelta completa\n", GRADOS_A_PASOS(360));
  DISPLAY_BT::printSerial(texto); DISPLAY_BT::flushSerial();
  while(salir == false){
    sprintf(texto,"Ingrese los pasos para girar una vuelta completa, iniciando con una g (ejemplo: g3300)\n");
    DISPLAY_BT::printSerial(texto); DISPLAY_BT::flushSerial();
    sprintf(texto,"o ingrese una G para terminar y guardar el ultimo valor de pasos \n");
    DISPLAY_BT::printSerial(texto); DISPLAY_BT::flushSerial();
    c = DISPLAY_BT::getCharSerial();
    if(c == 'g'){
      pasos = DISPLAY_BT::getIntSerial();
      sprintf(texto,"Se ingreso: %c%d\n",c,pasos);   DISPLAY_BT::printSerial(texto); DISPLAY_BT::flushSerial();
      gira_pasos(pasos);
    } 
    if(c == 'G'){
      sprintf(texto,"Se ingreso: %c\n",c);   DISPLAY_BT::printSerial(texto); DISPLAY_BT::flushSerial();
      FC_grados_pasos = ((float) pasos) / 360.0;
      preferencias.begin("Daniel1", false);
      preferencias.putFloat("FC_grados_pasos",FC_grados_pasos);
      preferencias.end();
      sprintf(texto,"Se guarda FC_grados_pasos=%f\n",FC_grados_pasos);   
      DISPLAY_BT::printSerial(texto); DISPLAY_BT::flushSerial();
      salir = true;
    }
  }
}
