#ifndef MACROS_DANIEL
 #include<macros.h>
#endif
#include<Boton.h>
#include<OrientacionPared.h>
#include<VL6180X_robot.h>
#include<DISPLAY_BT.h>
#include <ControlMov.h>

#define MAX_DATOS 100

// Inicializacion de atributos estaticos de la clase
float OrientacionPared::offset0=-30.0f, OrientacionPared::offset2=-30.0f;
float OrientacionPared::desv_estandar=1.0f;
int   OrientacionPared::dist_cal = 40;  // distancia de calibracion a pared en mm
int   OrientacionPared::dist_cal_tol =  10;  //tolerancia de calibracion a pared en mm
float OrientacionPared::umbral_dist_pared = UMBRAL_DIST_PARED;   // umbral para detectar que se acabo la pared en mm
int   OrientacionPared::mm[VL6180X_N];
float OrientacionPared::mmF[VL6180X_N];
char  OrientacionPared::texto[120];
ulong OrientacionPared::t_ms = 0;
int   OrientacionPared::L_mm0[MAX_DATOS]={0},
      OrientacionPared::L_mm1[MAX_DATOS]={0},
      OrientacionPared::L_mm2[MAX_DATOS]={0}, 
      OrientacionPared::L_indice=0;
Preferences OrientacionPared::preferencias;


void OrientacionPared::init(void){
  char texto[120];
  preferencias.begin("Daniel1", true); //abierto para lectura
  offset0  = preferencias.getFloat("offset0", -30.0f);
  offset2  = preferencias.getFloat("offset2", -30.0f);
  dist_cal = preferencias.getInt("dist_cal", 40);
  desv_estandar = preferencias.getFloat("desv_estandar",1.0f);
  preferencias.end();  
  sprintf(texto,"offset0 = %f, offset2 = %f, dist_cal =%d, desv_estandar = %f\n", 
                offset0, offset2, dist_cal, desv_estandar);
  DISPLAY_BT::printSerial(texto); DISPLAY_BT::flushSerial();
}

void OrientacionPared::determina_estado_sensores(byte *estado){
  static int mm[VL6180X_N];
  static float mmF[VL6180X_N];
  static ulong t_ms;
  int  I=0, F=0, D=0;
  VL6180X_robot::lecturasSensores(mm,mmF,&t_ms);
  if( mm[1] < umbral_dist_pared) I= 1<<2;
  if( mm[3] < umbral_dist_pared) F= 1<<1;
  if( mm[5] < umbral_dist_pared) D= 1;
  *estado = I | F | D;
}


/***********************************************************************
 * FUNCIONES AUXILIARES PARA SEGUIR PARED                              *
 ***********************************************************************/
void OrientacionPared::calculaPromediosSensores(float mmProm[VL6180X_N]){
  int i, j, n,  num_lecturas = 10;
  for(i = 0; i < VL6180X_N;    i++) mmProm[i]=0.0;
  for(i = 0; i < num_lecturas; i++) {
    n=VL6180X_robot::lecturasSensores(mm,mmF,&t_ms);
    for(j = 0; j < VL6180X_N;  j++) mmProm[j] += (float) mm[j];
  }
  for(i=0; i < VL6180X_N; i++) mmProm[i] /= (float) num_lecturas;
}

float OrientacionPared::calculaDesvEstandar(void){
  int i, num_lecturas = 30;
  float sum_x=0.0f, sum_x2 = 0.0f, dist, var, media;

  for(i = 0; i < num_lecturas; i++) {
    VL6180X_robot::lecturasSensores(mm,mmF,&t_ms);
    dist = mm[1];
    sum_x += dist; sum_x2 += dist * dist;
  }
  media = sum_x / num_lecturas;
  var = sum_x2/num_lecturas - media*media;
  return sqrtf(var);
}

void OrientacionPared::calcLineaDato(int mm0, int mm1, int mm2){
  if(L_indice  < MAX_DATOS) {
     L_mm0[L_indice] = mm0;
     L_mm1[L_indice] = mm1;
     L_mm2[L_indice] = mm2;
     L_indice++;
  }
}

void OrientacionPared::calcLinea1(int datos[], float dist_recorrida, float *m, float *b ){
  float sx=0,sy=0,sxy=0,sx2=0,x,y,inc_x = dist_recorrida/(L_indice-1.0f);
  int n; 
  char texto[80];
  n = L_indice;
  for(int i=0; i < n;i++){
    x = i*inc_x; y = datos[i]; 
    sx+=x; sy+=y; sxy+= x*y; sx2 += x*x;
  }
  *m = (n*sxy - sx*sy)/(n*sx2 - sx*sx);
  *b = (-sx*sxy + sx2*sy)/(n*sx2 - sx*sx);
//  sprintf(texto,"Recta a1=%f,a0=%f\n", *m,*b); DISPLAY_BT::printSerial(texto);
}

void OrientacionPared::calcLinea1(float X[], float Y[], int n, float *m, float *b ){
  float sx=0,sy=0,sxy=0,sx2=0, x, y;
  char texto[80];
  for(int i=0; i < n;i++){
    x = X[i]; y = Y[i]; 
    sx+=x; sy+=y; sxy+= x*y; sx2 += x*x;
  }
  *m = (n*sxy - sx*sy)/(n*sx2 - sx*sx);
  *b = (-sx*sxy + sx2*sy)/(n*sx2 - sx*sx);
}

// Calcula la mejor linea usando el algoritmo RANSAC
void OrientacionPared::calcLineaRS(float X[], float Y[], int np, float *pm, float *pb){
  int i, it, max_it = 12, p1, p2, n=0, n_mejor=0;
  float x[MAX_DATOS], y[MAX_DATOS], m, b, m_mejor=0.0f, b_mejor=0.0f, dif, max_desv=4.0f;

//  sprintf(texto,"RS Inicio np=%d\n",np);  DISPLAY_BT::printSerial(texto); 
  if (np == 2){
    calcLinea1(X,Y,np,pm,pb);
  } else {
    for(it=0; it < max_it && it < np; it++){
      p1 = random(0,np-1);  // punto 1 entre el valor de 0 y np-1
      do{
        p2 = random(0,np-1); // punto 2 entre el valor de 0 y np-1, diferente a p1
      } while(p1==p2);
      x[0] = X[p1]; y[0]=Y[p1]; x[1]=X[p2]; y[1] = Y[p2];
      calcLinea1(x,y,2,&m,&b);
//      sprintf(texto,"RS x0=%f y0=%f x1=%f y1=%f m=%f b=%f\n",x[0],y[0],x[1],y[1],m,b);
//      DISPLAY_BT::printSerial(texto); 
      n=0;
      for(i=0; i < np; i++){
        dif = fabs(Y[i] - (m*X[i] + b));
        if (dif < max_desv*desv_estandar) n++;
      }
      if (n > n_mejor) {m_mejor = m; b_mejor = b, n_mejor = n;}
    }
    n=0;
    for(i=0; i < np; i++){
      dif = fabs(Y[i] - (m_mejor*X[i] + b_mejor));
      if (dif < max_desv*desv_estandar) {x[n]=X[i]; y[n]=Y[i]; n++; };
    }
    calcLinea1(x,y,n,pm,pb);
  }
//  sprintf(texto,"RS np=%d n_mejor=%d m=%f b=%f\n",np, n, *pm, *pb);  DISPLAY_BT::printSerial(texto);
}

bool OrientacionPared::calcLinea(float dist_recorrida, float *ang, float *dist_pared_final, bool ransac){
  float sx=0,sy=0,sxy=0,sx2=0,x,y,inc_x = dist_recorrida/(L_indice-1.0f), m, b;
  float X[MAX_DATOS*3], Y[MAX_DATOS*3];
  int np=0; 
  bool res = false;
  char texto[80];
  for(int i=0; i < L_indice;i++){
    x = i*inc_x - 60; y = L_mm0[i] + offset0; 
    if (y < UMBRAL_DIST_PARED) { X[np] = x; Y[np] = y; np++;}
    x = i*inc_x     ; y = L_mm1[i]; 
    if (y < UMBRAL_DIST_PARED) { X[np] = x; Y[np] = y; np++;}
    x = i*inc_x + 60; y = L_mm2[i] + offset2; 
    if (y < UMBRAL_DIST_PARED) { X[np] = x; Y[np] = y; np++;}
  }
  if (np >= 2){
    if(ransac)  calcLineaRS(X,Y,np, &m, &b);
    else        calcLinea1 (X,Y,np, &m, &b);
    *ang = atanf(m)* 180.0/PI;
    *dist_pared_final = m*dist_recorrida + b;
    res = true;
//  sprintf(texto,"ang=%f,a0=%f\n", ang,b); DISPLAY_BT::printSerial(texto);
  }
  L_indice = 0;
  return res;
}

void OrientacionPared::calibra(Boton &botonDaniel){
  int i, pasos_izq0, pasos_izq1, pasos_izq2, recorrido_cal=100, inc_pasos = MM_A_PASOS(recorrido_cal);
  bool continuar = true;
  float dist_recorrida, m0, m1, m2, b0, b1, b2, mmProm[VL6180X_N], ang, dist_pared_final;
  char texto[80];

  botonDaniel.ledON();
  do{
    calculaPromediosSensores(mmProm);
    sprintf(texto, "Pared\nizquierda\ndist=%4.1f\nOprimaBot",mmProm[1]);
    DISPLAY_BT::print(texto,2);
  } while(botonDaniel.botonOprimido() == false); 
  DISPLAY_BT::print("\n\n\n\n",2);
  botonDaniel.ledOFF(); delay(1000); 
  dist_cal = (int) mmProm[1];
  desv_estandar = calculaDesvEstandar();
  sprintf(texto,"dist=%3d\ndStd=%4.1f\n\nOprimaBot",dist_cal, desv_estandar);
  DISPLAY_BT::print(texto,2);
  botonDaniel.esperaOprimir();  delay(1000);

  ControlMov::avanza_vel();
  delay(100);
  pasos_izq0 = ControlMov::getPasosIzq();
  do {
    VL6180X_robot::lecturasSensores(mm,mmF,&t_ms);
    calcLineaDato(mm[0],mm[1],mm[2]);
    pasos_izq1 = ControlMov::getPasosIzq();
  } while (pasos_izq1 - pasos_izq0 < inc_pasos);
  ControlMov::para();
  dist_recorrida =  PASOS_A_MM(pasos_izq1 -pasos_izq0);  
  calcLinea1(L_mm0, dist_recorrida, &m0, &b0);
  calcLinea1(L_mm1, dist_recorrida, &m1, &b1);
  calcLinea1(L_mm2, dist_recorrida, &m2, &b2);
  calcLinea(dist_recorrida, &ang, &dist_pared_final, false);
  ControlMov::gira_grados(ang);
  offset0 = b1 - b0; offset2 = b1 - b2;  // ajuste al sensor central s1
  sprintf(texto,"offset0=%f, offset2=%f dist_cal=%d ang=%3.1f", offset0,offset2,dist_cal,ang);
  DISPLAY_BT::printSerial(texto);
  sprintf(texto,"of0=%6.2f\nof2=%6.2f\nd_cal=%3d\nang=%5.1f",   offset0,offset2,dist_cal,ang);
  DISPLAY_BT::print(texto, 2);
  DISPLAY_BT::flushSerial();
  preferencias.begin("Daniel1", false); //abierto para lectura - escritura
  preferencias.putFloat("offset0",offset0);
  preferencias.putFloat("offset2",offset2);
  preferencias.putInt("dist_cal",dist_cal);
  preferencias.putFloat("desv_estandar",desv_estandar);
  preferencias.end();
  L_indice = 0;
  botonDaniel.esperaOprimir(); delay(1000);
}

bool OrientacionPared::en_rango_dist(float dist_pared_mm){
  return (dist_pared_mm >= dist_cal - dist_cal_tol) && (dist_pared_mm <= dist_cal + dist_cal_tol);
}

float OrientacionPared::ajustePorCercania(float dist_pared) {
  float ang=0.0f; 
  if(en_rango_dist(dist_pared) == false) 
     ang = (dist_pared - dist_cal > 0)? 3.0f:-3.0f;
  return ang;
}

void OrientacionPared::alineaPared(void){
  float ang_laser, ang_ajuste_dist=0.0f, ang_deg, dist_pared_final;
  int i, k=0;

  VL6180X_robot::lecturasSensores(mm,mmF,&t_ms);
  do{
    for (i=0; i < 6; i++){
      VL6180X_robot::lecturasSensores(mm,mmF,&t_ms);
      calcLineaDato(mm[0],mm[1],mm[2]);
    }
    calcLinea(0.0f, &ang_laser, &dist_pared_final, false);
    ControlMov::gira_grados(ang_laser); 
    k++;
  } while (k < 3 && fabs(ang_laser) > 2.0f); 
  if (mmF[5] < UMBRAL_DIST_PARED) dist_cal = (mmF[1]+mmF[5])/2.0;   // ajusta dist_cal si hay pared izquierda
  
  ang_ajuste_dist = ajustePorCercania(dist_pared_final);
  ControlMov::gira_grados(ang_ajuste_dist);
//  sprintf(texto, "d_cal=%d\nang=%3.1f\ndist=%4.0f\n",dist_cal,ang_ajuste_dist, dist_pared_final);
//  DISPLAY_BT::print(texto,2);  delay(3000);
}

float OrientacionPared::avanzaPared(bool pared_izq, int vel, int aceleracion) {
  int pasos_ajuste, dist_pared_int, umbral_frente=100, i;
  float ang_laser, ang_ajuste_dist=0.0f, ang_deg;
  float dist_pared_final, dist_ajuste;
  float distancia_control = 30; // cada 30mm de recorrido de la rueda izquierda se hace un ajuste de control
  int pasos_izq_ini, pasos_izq0, pasos_izq1, inc_pasos = MM_A_PASOS(distancia_control); // ajuste de orientacion cada 30mm
  bool continuar = true; 
  char texto[80];

  alineaPared();
  ControlMov::avanza_vel();
  pasos_izq_ini = pasos_izq0 = ControlMov::getPasosIzq();
  do{
    VL6180X_robot::lecturasSensores(mm,mmF,&t_ms);
    if(pared_izq) dist_pared_int = mm[1];
    else          dist_pared_int = mm[5];
    calcLineaDato(mm[0],mm[1],mm[2]);
    pasos_izq1 = ControlMov::getPasosIzq();
//    sprintf(texto, "pasos_izq1=%d mm[1]=%d\n", pasos_izq1, mm[1]);    DISPLAY_BT::printSerial(texto); 
    if(pasos_izq1 - pasos_izq0 >= inc_pasos) { // corrige orientacion
      dist_ajuste = PASOS_A_MM(pasos_izq1 -pasos_izq0);
      if(calcLinea(dist_ajuste, &ang_laser, &dist_pared_final, true)) {
        ang_ajuste_dist = ajustePorCercania(dist_pared_final);
        ang_deg = ang_laser + ang_ajuste_dist;
//        sprintf(texto, "dist_pared_final=%f, ang_deg=%f\n",dist_pared_final, ang_deg);     DISPLAY_BT::printSerial(texto);
        pasos_ajuste = ControlMov::conv_grados_a_pasos(ang_deg) * 2;
        ControlMov::SetOffsetEncD(-pasos_ajuste);
      }
      pasos_izq0 = pasos_izq1;
    }
    if(pared_izq) continuar = mm[1] < umbral_dist_pared && mm[3] >= umbral_frente; 
    else          continuar = mm[5] < umbral_dist_pared && mm[3] >= umbral_frente; 
  } while (continuar);  
  if(mm[3] <= umbral_frente) {
//  ControlMov::avanza_pasos(MM_A_PASOS(mmF[3] - dist_cal));
    ControlMov::avanza_vel(12);
    do {
      VL6180X_robot::lecturasSensores(mm,mmF,&t_ms);
    } while (mm[3] > dist_cal+55);
    ControlMov::avanza_pasos(MM_A_PASOS(30));
  } else                                           // Se acabo pared izquierda
    ControlMov::avanza_pasos(MM_A_PASOS(44+dist_cal));
  if(ang_ajuste_dist!=0.0f)
    pasos_ajuste = ControlMov::gira_pasos(-ang_ajuste_dist);

  VL6180X_robot::lecturasSensores(mm,mmF,&t_ms);
/*
  ControlMov::gira_grados(-90.0f);
  sprintf(texto, "mm[1]=%d\n",mm[1]);
  DISPLAY_BT::print(texto,2);  delay(2000);
  DISPLAY_BT::flushSerial();
  while(true) ; 
*/
  return mm[1];
}