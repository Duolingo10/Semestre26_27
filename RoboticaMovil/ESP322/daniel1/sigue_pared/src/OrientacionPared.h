#include <Preferences.h>

class OrientacionPared{
 private:
   static int mm[];
   static float mmF[];
   static char texto[];
   static ulong t_ms;
   static float L_mm0[],L_mm1[],L_mm2[];
   static int   L_indice;
   static float offset0, offset2, desv_estandar;
   static Preferences preferencias;
   static int avanzaParedFrente(void);

 public:
  static int   dist_cal, dist_cal_tol;
  static void  determina_estado_sensores(byte *estado);
  static float avanzaPared(bool pared_izquierda, int vel=VEL_BAJA, int aceleracion=ACEL_BAJA);
  static void  alineaPared(void);
  static bool  en_rango_dist(float dist_pared);
  static float ajustePorCercania(float dist);
  static void calculaPromediosSensores(float mmProm[]);
  static float calculaDesvEstandar(void);
  static void calcLineaDato(int mm0, int mm1, int mm2);
  static void calcLineaDato(float mm0F, float mm1F, float mm2F);
  static void calcLinea1(float datos[],float dist_recorrida,float offsetX_sensor,float *m, float *b);
  static void calcLinea1(float X[], float Y[], int n, float *m, float *b );
  static void calcLineaRS(float X[], float Y[], int n, float *m, float *b);
  static bool calcLinea(float dist_recorrida, float *ang, float *dist_pared_final, bool ransac,
                bool m0, bool m1, bool m2);
  static void calibra(Boton &botonDaniel);
  static void init(void);
};