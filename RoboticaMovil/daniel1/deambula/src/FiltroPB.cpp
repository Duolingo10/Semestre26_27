#include <FiltroPB.h>

bool FiltroPB::init(int lectura, float a) {
   salidaFiltro = (float) lectura;
   alpha = a;
   return true;
}

float FiltroPB::procesa(int lectura) {
  salidaFiltro = alpha * lectura + (1.0 - alpha ) * salidaFiltro;
  return  salidaFiltro;
}
