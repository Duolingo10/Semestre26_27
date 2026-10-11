#include <Preferences.h>

class OrientacionPared{
 private:
   static int mm[];
   static float mmF[];
   static char texto[];
   static ulong t_ms;
   static int   L_mm0[],L_mm1[],L_mm2[], L_indice;
   static float offset0, offset2, desv_estandar;
   static Preferences preferencias;

 public:
  static int   dist_cal, dist_cal_tol;
  static float umbral_dist_pared;
  static void  determina_estado_sensores(byte *estado);
  static float avanzaPared(bool pared_izquierda, int vel=VEL_BAJA, int aceleracion=ACEL_BAJA);
  static void  alineaPared(void);
  static bool  en_rango_dist(float dist_pared);
  static float ajustePorCercania(float dist);
  static void calculaPromediosSensores(float mmProm[]);
  static float calculaDesvEstandar(void);
  static void calcLineaDato(int mm0, int mm1, int mm2);
  static void calcLinea1(int datos[],float dist_recorrida, float *m, float *b);
  static void calcLinea1(float X[], float Y[], int n, float *m, float *b );
  static void calcLineaRS(float X[], float Y[], int n, float *m, float *b);
  static bool calcLinea(float dist_recorrida, float *ang, float *dist_pared_final, bool ransac);
  static void calibra(Boton &botonDaniel);
  static void init(void);
};