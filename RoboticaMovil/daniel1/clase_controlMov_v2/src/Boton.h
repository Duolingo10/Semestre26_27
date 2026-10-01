#include <Preferences.h>
#include <macros.h>

class Boton {
 private:
   int pin_boton, pin_led;
   float factor_conversion_volts;
   Preferences preferencias;
 public:
   bool init(int boton, int led);
   int leeOpcion(int num_opciones);
   void esperaEstado(int estado);
   void parpadear(int veces);
   float nivelBateria(int *lectura);
   void ledON(void);
   void ledOFF(void);
   float bateria(void);
   float calibra_bat(void);
   void esperaOprimir(void);
   void esperaLiberar(void);
};