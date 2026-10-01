#include <Arduino.h>
#include <macros.h>
#include <DISPLAY_BT.h>
#include <ControlMov.h>
#include <Boton.h>
#include <VL6180X_robot.h>
#include <Configura.h>
#include <Deambula.h>
#include <SiguePared.h>
#include <OrientacionPared.h>

ControlMov movDaniel;
Boton      botonDaniel;
VL6180X_robot sensoresDaniel;

int   n, n0, mm[VL6180X_N]={0};
float mmF[VL6180X_N]={0};
unsigned long tiempo_ms; 
char  texto[80];


void setup(){
  Serial.begin(115200);
  delay(2000);
  botonDaniel.init(PIN_BOTON,PIN_LED);
  while(DISPLAY_BT::init_SSD1306()==false) delay(1000);
  movDaniel.initMotores(PIN_PWMA,PIN_AIN2,PIN_AIN1,PIN_STBY,PIN_BIN1,PIN_BIN2,PIN_PWMB);
  movDaniel.initEncoders(PIN_ENCB_IZQ, PIN_ENCA_IZQ, PIN_ENCB_DER, PIN_ENCA_DER);
  OrientacionPared::init();
  movDaniel.activaMotores();
  sensoresDaniel.init(PIN_SHUT0,PIN_SHUT1,PIN_SHUT2,PIN_SHUT3,PIN_SHUT4,PIN_SHUT5,
                      PIN_SHUT6,PIN_INT0, PIN_INT1, PIN_INT2, PIN_INT3);
  sensoresDaniel.iniciaTareaLecturasContinuas(); delay(100);
  n0 = VL6180X_robot::lecturasSensores(mm,mmF,&tiempo_ms);
  delay(1000);
  n  = VL6180X_robot::lecturasSensores(mm,mmF,&tiempo_ms);
  sprintf(texto,"DANIEL\nsensores\nlaser\n%d m/s", n - n0); 
  DISPLAY_BT::print(texto,2); 
  delay(3000);
}

void ver_sensores(void){
  char texto[50];
  while(true) {
    VL6180X_robot::lecturasSensores(mm, mmF, &tiempo_ms);
    sprintf(texto,"  %3d(%3d)\n %3d  %3d\n%3d    %3d\n %3d  %3d",
      mm[3],mm[7],mm[2],mm[4],mm[1],mm[5],mm[0],mm[6]);
    DISPLAY_BT::print(texto,2);  
    delay(500);
  }
}

void ver_bateria(void){
  float voltaje; 
  DISPLAY_BT::print4("BATERIA", "mantenga", "boton", "oprimido "); delay(1000);
  voltaje =  botonDaniel.bateria();           
  sprintf(texto, "%4.2f", voltaje);
  DISPLAY_BT::print4("Voltaje","bateria:", texto, " "); delay(3000);
}

void loop() {
  char op[6][20]={"0:SiguePar","1:Deambula","2:Sensores", "3:Bateria","4:Giro360","5:Avanza"},
       texto[80];
  int opcion=0;
  
  movDaniel.calibra_frenado();
  DISPLAY_BT::print4(op[0],op[1],op[2],op[3]); delay(3000);
  opcion = botonDaniel.leeOpcion(5);
  if(opcion == -1) {
    sprintf(texto,"Opcion\n-1:Config\nlibere\nboton");
    DISPLAY_BT::print(texto,2);  delay(1000);
    botonDaniel.esperaLiberar(); delay(1000);
  } else {
    sprintf(texto,"Opcion\n%s\noprima\nboton",op[opcion]);
    DISPLAY_BT::print(texto,2);  delay(1000);
    botonDaniel.esperaOprimir(); delay(1000);
  }
  switch (opcion)  {
    case -1: Configura::menu(botonDaniel,movDaniel); break;
    case  0: SiguePared::sigueParedIzq(); break;
    case  1: Deambula::run();              break;
    case  2: ver_sensores();               break;
    case  3: ver_bateria ();               break;
    case  4: ControlMov::gira_grados(360.0f); delay(3000);
             ControlMov::gira_grados(360.0f); delay(3000);
             ControlMov::gira_grados(360.0f); delay(3000); break;
    case  5: ControlMov::avanza_pasos(MM_A_PASOS(200)); delay(1000);
             ControlMov::gira_grados(90.0f); delay(1000);
             ControlMov::gira_grados(-90.0f); delay(1000);
             ControlMov::retrocede_pasos(MM_A_PASOS(200)); delay(1000); break;
    default:
    break;
  }
}