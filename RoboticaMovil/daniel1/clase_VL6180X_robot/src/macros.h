#define IIC_BPS_H (400000UL)       // Velocidad del canal IIC en bit por segundo
#define IIC_BPS_L (400000UL)       // Velocidad del canal IIC en bit por segundo
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

// *** Sensores laser VL6180X   ***
		     // VIN se conecta a 3.3V+
		     // GND se conecta a GND
#define PIN_SDA     21 // SDA se conecta a D21 en ESP32
#define PIN_SCL     22 // SCL se conecta a D22 en ESP32
#define PIN_SHUT0   18 // Pin SHUT sensor izquierdo atras
#define PIN_SHUT1    5 // Pin SHUT sensor izquierdo
#define PIN_SHUT2   17 // Pin SHUT sensor izquierdo adelante
#define PIN_SHUT3   16 // Pin SHUT sensor frontal
#define PIN_SHUT4   4  // Pin SHUT sensor derecho adelante
#define PIN_SHUT5   32 // Pin SHUT sensor derecho
#define PIN_SHUT6   27 // Pin SHUT sensor derecho atras (compartido con PIN_STBY)
#define PIN_INT0    38 // Pin INT  sensor izquierdo atras
#define PIN_INT1    39 // Pin INT  sensor izquierdo
#define PIN_INT2    34 // Pin INT  sensor izquierdo adelante
#define PIN_INT3    35 // Pin INT  sensor frontal

// Parametros de la generacion de pulsos PWM para controlar los dos motores
#define PWM_CANAL_A    0
#define PWM_CANAL_B    1
#define PWM_FRECUENCIA 20000
#define PWM_RESOLUCION 8
	
#define VL6180X_N       8 // Numero de sensores laser
#define VL6180X_N_SHUT  7 // Numero de sensores con pin SHUT (0,1,...N_SHUT-1)
#define VL6180X_N_INT   4 // Número de sensores con INT (0,1,...,N_INT-1)