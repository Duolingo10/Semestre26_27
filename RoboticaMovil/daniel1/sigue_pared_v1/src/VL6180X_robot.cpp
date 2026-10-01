#include <Arduino.h>
#include <VL6180X_robot.h>
#include <FiltroPB.h>
#include <DISPLAY_BT.h>

SemaphoreHandle_t VL6180X_robot::mutex_sensores;
int VL6180X_robot::pin_SHUT[VL6180X_N_SHUT]={0};
int VL6180X_robot::pin_INT[VL6180X_N_INT]={0};
bool VL6180X_robot::primera_vez = true;
volatile int VL6180X_robot::cnt = 0;
volatile unsigned long VL6180X_robot::tiempo_ms = 0;
volatile int VL6180X_robot::lecturas_mm[VL6180X_N] ={0};
volatile float VL6180X_robot::lecturasFiltros_mm[VL6180X_N] = {0.0};
volatile float VL6180X_robot::alphas[VL6180X_N]={.3,.3,.3,.5,.3,.3,.3,.3};
int VL6180X_robot::leer[VL6180X_N]={true,true,true,true,false,true,false,false};  
VL6180X VL6180X_robot::sensores[VL6180X_N];
 
void VL6180X_robot::init(int SHUT0,int SHUT1,int SHUT2,int SHUT3,int SHUT4,int SHUT5,int SHUT6,
                         int INT0, int INT1, int INT2, int INT3){
  char texto[80];
  Wire.begin(PIN_SDA, PIN_SCL, 200000L);  // PIN_SDA, PIN_SCL, Frecuencia
  pin_SHUT[0]=SHUT0; pin_SHUT[1]=SHUT1; pin_SHUT[2]=SHUT2; pin_SHUT[3]=SHUT3; pin_SHUT[4]=SHUT4;
  pin_SHUT[5]=SHUT5; pin_SHUT[6]=SHUT6;
  pin_INT[0]=INT0; pin_INT[1]=INT1; pin_INT[2]=INT2; pin_INT[3]=INT3;
  for(int i=0; i< VL6180X_N_SHUT; i++){
    pinMode(pin_SHUT[i],OUTPUT);  digitalWrite(pin_SHUT[i],LOW); 
  }
  for(int i=0; i< VL6180X_N_INT; i++) pinMode(pin_INT[i],INPUT_PULLUP); 
  sprintf(texto,"sensor %d\n", VL6180X_N-1); DISPLAY_BT::printSerial(texto);
  configura_sensor(&sensores[VL6180X_N-1], 0x31, 1); /*sensor Abajo siempre activo*/ 
  for(int i=0; i < VL6180X_N_SHUT; i++){
    digitalWrite(pin_SHUT[i], HIGH); 
    sprintf(texto,"sensor %d\n", i); DISPLAY_BT::printSerial(texto);
    if(i == 3)
       configura_sensor(&sensores[i],0x32+i, 2); // escala=2 sensor frontal
    else
       configura_sensor(&sensores[i],0x32+i, 1);
  }
  configura_sensor(&sensores[VL6180X_N-1], 0x29, 1);;   // default 0x29 
  mutex_sensores = xSemaphoreCreateMutex();
  xSemaphoreGive(mutex_sensores);
}

void VL6180X_robot::configura_sensor(VL6180X *sensor, byte direccion, byte escala) {
  delay(100);
  sensor->init();
  delay(10);
  sensor->configureDefault(); 
  sensor->setAddress(direccion);  
  sensor->setScaling(escala);
  sensor->setTimeout(500);
  sensor->writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 12);    // 12 ms
  sensor->writeReg(VL6180X::SYSTEM__MODE_GPIO1, 0x10); // Habilita pin INT en LOW
  sensor->writeReg(VL6180X::SYSTEM__INTERRUPT_CONFIG_GPIO,0x04); // Cuando muestra lista
  sensor->writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x03); // borra interrupcion
  delay(10);
}

void VL6180X_robot::loop(void *parameter) {
  FiltroPB filtros[VL6180X_N];
  int i, mm[VL6180X_N]={0};
  bool fin_mediciones, fin_sensor[VL6180X_N], timeout=false;
  unsigned long t0, t;
  char texto[80];

  while(true){
    fin_mediciones=false; 
    for(i=0; i < VL6180X_N; i++) {
      fin_sensor[i] = false;
      if(leer[i]){
        sensores[i].writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x07);
        sensores[i].writeReg(VL6180X::SYSRANGE__START, 0x01);   // inicia medicion;
      }
    }
    t0 = millis();
    for(i=0; i < 5; i++) {
      delay(1); // cede el control
    }
    while(fin_mediciones == false && (timeout=(millis() - t0 < 100))) {
      for(i=0; i < VL6180X_N; i++)
        if(leer[i] && (fin_sensor[i] == false) && (digitalRead(pin_INT[i]) == LOW)){  // lectura lista
          mm[i] = sensores[i].readReg(VL6180X::RESULT__RANGE_VAL); 
          sensores[i].writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x07);
          fin_sensor[i] = true;
        }
      for (i=VL6180X_N_INT; i < VL6180X_N; i++)
        if((fin_sensor[i] == false) && 
          ((sensores[i].readReg(VL6180X::RESULT__INTERRUPT_STATUS_GPIO) & 0x04) != 0) ) { 
          mm[i] = sensores[i].readReg(VL6180X::RESULT__RANGE_VAL); 
          sensores[i].writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x01);
          fin_sensor[i] = true;
        }
      fin_mediciones = true;
      for(i=0; i <VL6180X_N; i++){
        if(leer[i] && (fin_sensor[i] == false))
          fin_mediciones = false;
      }
      if(fin_mediciones == false)
        delay(1); // cede el control
    }
    if(timeout == false) {
      DISPLAY_BT::printSerial("*** timeout ***\n");
      for(int i = 0; i < VL6180X_N; i++){
        sprintf(texto,"fin_sensor:%d,%d,%d,%d,%d,%d,%d,%d\n",fin_sensor[0],fin_sensor[1],fin_sensor[2],
        fin_sensor[3],fin_sensor[4],fin_sensor[5],fin_sensor[6],fin_sensor[7]);
        DISPLAY_BT::printSerial(texto);
      }
    }
    mm[3] *= 2;  // correcion por factor escala = 2 en sensor Frontal
    if(primera_vez) {
      for(i=0;i<VL6180X_N; i++)
        filtros[i].init(mm[i], alphas[i]);
      primera_vez = false;
    }
    xSemaphoreTake(mutex_sensores, portMAX_DELAY);
    for(i=0; i<VL6180X_N; i++){
      lecturas_mm[i]        = mm[i];
      lecturasFiltros_mm[i] = filtros[i].procesa(mm[i]);
    }
    cnt++;  //incrementa el contador de mediciones
    tiempo_ms = millis();
    xSemaphoreGive(mutex_sensores);
  }
}

void VL6180X_robot::iniciaTareaLecturasContinuas(void){
  static TaskHandle_t tarea;
  int nucleo=0, prioridad = 1;
  xTaskCreatePinnedToCore(VL6180X_robot::loop,"Sensores",10000,NULL,prioridad,&tarea,nucleo);
}

void VL6180X_robot::reiniciaFiltros(void){
  primera_vez = true;
}

int VL6180X_robot::lecturasSensores(int mm[], float mmF[], unsigned long * t_ms) {
    static int i, n=0, espera;
    do {
      xSemaphoreTake(mutex_sensores, portMAX_DELAY);
      espera = (n == cnt)? 1 : 0;
      xSemaphoreGive(mutex_sensores);
      if(espera) delay(3);
    } while(espera == 1);
    xSemaphoreTake(mutex_sensores, portMAX_DELAY);
    for(i=0; i<VL6180X_N; i++){
      mm[i]  = lecturas_mm[i];
      mmF[i] = lecturasFiltros_mm[i];
    }
    *t_ms = tiempo_ms;
    n = cnt;
    xSemaphoreGive(mutex_sensores);
    return n;  // regresa el numero de medicion
}