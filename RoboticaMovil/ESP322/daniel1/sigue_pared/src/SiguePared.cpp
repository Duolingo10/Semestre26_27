#include<Arduino.h>
#include<Boton.h>
#include<SiguePared.h>
#include<ControlMov.h>
#include<VL6180X_robot.h>
#include<OrientacionPared.h>
#include<DISPLAY_BT.h>

#define B(i,f,d) ((i)<<2 | (f<<1) | (d))    // convierte a binario

int SiguePared::mm[VL6180X_N] = {0};
float SiguePared::mmF[VL6180X_N]={0}, SiguePared::mmProm[VL6180X_N]={0};
ulong SiguePared::tiempo_ms = 0; 
char SiguePared::texto[80]={0};

void SiguePared::sigueParedIzq(void){
  byte edo_sensores;
  int dist_paredI=OrientacionPared::dist_cal;
  char texto[80];
  delay(10);
  while(true){
    VL6180X_robot::lecturasSensores(mm,mmF,&tiempo_ms);
    OrientacionPared::determina_estado_sensores(&edo_sensores);
//    sprintf(texto, "edo =%d\nd_i=%d\n",edo_sensores,mm[1]); DISPLAY_BT::print(texto,2); delay(1000);
    switch(edo_sensores){
     case B(0,0,0): 
     case B(0,0,1):
     case B(0,1,0):   
     case B(0,1,1): ControlMov::gira_grados(91.0f);
                    ControlMov::avanza_pasos(MM_A_PASOS(90+(GROSOR_PARED_MM)+dist_paredI)); //90 (robot) + 20 (pared) + 40
                    VL6180X_robot::lecturasSensores(mm,mmF,&tiempo_ms);
                    if(mm[2]> UMBRAL_DIST_PARED_MM)
                      ControlMov::avanza_pasos(MM_A_PASOS(50 + OrientacionPared::dist_cal)); 
                    sprintf(texto,"dist=%d\n", mm[1]); DISPLAY_BT::print(texto,2);  
                    break; // sin pared izquierda
     case B(1,0,1):
     case B(1,0,0): dist_paredI = OrientacionPared::avanzaPared(true);
                    break;
     case B(1,1,0): ControlMov::gira_grados(-90.0);  break;
     case B(1,1,1): ControlMov::gira_grados(-180.0); break;
     default      : break;
    }
  }
}