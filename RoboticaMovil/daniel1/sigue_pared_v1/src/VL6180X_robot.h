#include <VL6180X.h>
#include <macros.h>

class VL6180X_robot {
  private:
    static int pin_SHUT[VL6180X_N_SHUT];
    static int pin_INT[VL6180X_N_INT];
    static SemaphoreHandle_t mutex_sensores;
    static bool primera_vez;
    volatile static int cnt;
    volatile static unsigned long tiempo_ms;
    volatile static int lecturas_mm[VL6180X_N];
    volatile static float lecturasFiltros_mm[VL6180X_N];
    volatile static float alphas[VL6180X_N];
    static int leer[VL6180X_N];
    static int escala[VL6180X_N];
    static int pin_ints[VL6180X_N];
    static VL6180X sensores[VL6180X_N];
    void configura_sensor(VL6180X * sensor, byte direccion, byte escala);
    static void loop(void *parameter);
  public:
    void init(int SHUT0,int SHUT1,int SHUT2,int SHUT3,int SHUT4,int SHUT5, int SHUT6,
              int INT0, int INT1, int INT2, int INT3);
    void iniciaTareaLecturasContinuas(void);
    static int lecturasSensores(int mm[], float mmF[], unsigned long * t_ms);
    static void reiniciaFiltros(void);
};