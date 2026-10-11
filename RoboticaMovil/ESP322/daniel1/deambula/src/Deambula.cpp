#include<Deambula.h>
#include<ControlMov.h>
#include<DISPLAY_BT.h>
#include<VL6180X_robot.h>

#define B(i,f,d) ((i)<<2 | (f<<1) | (d))    // convierte a binario

byte determina_estado_sensores(void){
  static int umbral_dist_pared_lado = 100; // 100mm umbral para detectar pared
  static int umbral_dist_pared_frente = 150; // 100mm umbral para detectar pared
  static int mm[VL6180X_N];
  static float mmF[VL6180X_N];
  static ulong t_ms;
  char texto[120];
  byte estado = 0;
  int  I=0, F=0, D=0;
  VL6180X_robot::lecturasSensores(mm,mmF,&t_ms);
  if( mm[1] < umbral_dist_pared_lado)   I= 1<<2;  // Sensor izquierdo
  if( mm[3] < umbral_dist_pared_frente) F= 1<<1;  // Sensor frontal 
  if( mm[5] < umbral_dist_pared_lado)   D= 1;     // Sensor derecho
  estado = I | F | D;
//  sprintf(texto, "  %3d\n%3d  %3d\n\nestado=%d", mm[3],mm[1],mm[5],estado);
//  DISPLAY_BT::print(texto,2);
  return estado;
}

void Deambula::run(void){
  static byte estado, vel=20;
  int grados;
  while(true) {
    ControlMov::avanza_vel(vel);
    estado = determina_estado_sensores();
    if(estado != 0) ControlMov::para();
    switch (estado){    
      case B(0,0,0) : break;
      case B(0,0,1) : grados = random(45,90);
                      ControlMov::gira_pasos(GRADOS_A_PASOS(grados),vel);
                      ControlMov::avanza_pasos(MM_A_PASOS(30),vel); 
                      break;
      case B(0,1,1) : ControlMov::gira_pasos(GRADOS_A_PASOS(90.0),vel);
                      break;
      case B(1,0,0) : grados = random(45,90);
                      ControlMov::gira_pasos(GRADOS_A_PASOS(-grados),vel);
                      ControlMov::avanza_pasos(MM_A_PASOS(30),vel); 
                      break;
      case B(1,1,0) : ControlMov::gira_pasos(GRADOS_A_PASOS(-90.0),vel);
                      break;
      case B(0,1,0) :
      case B(1,0,1) :
      case B(1,1,1) : ControlMov::gira_pasos(GRADOS_A_PASOS(180.0),vel);
                      break;
      default:        break;
    }
    delay(1);
  }
}