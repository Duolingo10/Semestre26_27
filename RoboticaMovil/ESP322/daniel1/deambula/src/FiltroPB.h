class FiltroPB{
private:
  float salidaFiltro=0.0;
  float alpha = 0.3;
public:
   bool init(int lectura=0, float alpha=0.3);
   float  procesa(int lectura);
};  