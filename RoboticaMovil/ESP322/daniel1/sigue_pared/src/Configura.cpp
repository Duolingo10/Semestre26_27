#include <Boton.h>
#include <ControlMov.h>
#include <Configura.h>
#include <DISPLAY_BT.h>
#include <VL6180X_robot.h>
#include <OrientacionPared.h>

void Configura::menu(Boton &botonDaniel, ControlMov &movDaniel){
  char texto[20], robot_ID[80], op[8][20] = {  
    "-1=fijaIdRobot ","0=MuestraRobotID","1=activaBluetooth","2=calibraBat",
    "3=calibraPWMmot","4=estimaKpKdKi",  "5=calibraGiro360", "6=calibraPared"};
  while(true){
    int opcion=0;
    DISPLAY_BT::print8(op[0],op[1],op[2],op[3],op[4],op[5],op[6],op[7]);delay(3000);
    opcion = botonDaniel.leeOpcion(6);
    if(opcion == -1) {
      sprintf(texto,"%s\nlibere\nboton",op[opcion+1]);
      DISPLAY_BT::print(texto,2);  delay(1000);
      botonDaniel.esperaLiberar(); delay(1000);
    } else {
      sprintf(texto,"%s\noprima\nboton",op[opcion+1]);
      DISPLAY_BT::print(texto,2);  delay(1000);
      botonDaniel.esperaOprimir(); delay(1000);
    }
    switch (opcion)  {
      case -1: ControlMov::set_robot_ID();                   break;
      case  0: ControlMov::get_robot_ID(robot_ID);
               DISPLAY_BT::print4("robot ID","", robot_ID,""); 
               delay(3000); break;
      case  1: ControlMov::get_robot_ID(robot_ID);
               DISPLAY_BT::init_BT(robot_ID); delay(1000);   break;
      case  2: botonDaniel.calibra_bat();                    break;
      case  3: movDaniel.calibra_PWM_motores(botonDaniel);              break;
      case  4: movDaniel.estimaKpKdKi();                     break;
      case  5: movDaniel.calibra_giro();                     break;
      case  6: OrientacionPared::calibra(botonDaniel);       break;
      default: break;
    }
  } 
}