#define IIC_BPS_H (400000UL)       // Velocidad del canal IIC en bit por segundo
#define IIC_BPS_L (100000UL)       // Velocidad del canal IIC en bit por segundo
#define CONV_BAT (3.6/1100.0)  // Factor para convertir lectura 12 bits (0-4095) a volts
#define LILYGO_OLED  

#define PIN_LED    2 
#define PIN_BOTON 15 
// Conexiones del LILYGO OLED V3 al driver TB6612FNG
//   PIN DRV   --->  PIN ESP32
#define PIN_PWMA 33  //   Lado izquierdo del TB6612FNG
#define PIN_AIN2 25  
#define PIN_AIN1 26  
#define PIN_STBY 27  
#define PIN_BIN1 14  
#define PIN_BIN2 12   
#define PIN_PWMB 13  

// *** Lado derecho del driver TB6612 ***
//      VM          Fuente potencia de 5v+
//      VCC         Marcado como 3V3 (3.3v) en el ESP32
//      A01         Hacia el motor izq M2        
//      A02         Hacia el motor izq M1
//      B02         Hacia el motor der M2
//      B01         Hacia el motor der M1
//      GND         Hacia el pin GND en el ESP32

// Conexiones de los encoders de los motores
#define PIN_ENCB_IZQ 36 // motor izq C1 cable verde
#define PIN_ENCA_IZQ 37 // motor izq C2 cable amarillo
#define PIN_ENCB_DER 19 // motor der C2 cable amarillo
#define PIN_ENCA_DER 23 // motor der C1 cableverde
//                         VCC se conecta a 3v3 (3.3v en el ESP32)
//                         GND se conecta a GND en el ESP32

// Parametros de la generacion de pulsos PWM para controlar los dos motores
#define PWM_CANAL_A    0
#define PWM_CANAL_B    1
#define PWM_FRECUENCIA 20000
#define PWM_RESOLUCION 8

