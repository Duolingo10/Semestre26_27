class Motores {
 private:
   int pin_PWMA, pin_AIN2, pin_AIN1, pin_STBY, pin_BIN1, pin_BIN2, pin_PWMB;
 public:
   bool init(int PWMA, int AIN2, int AIN1, int STBY, int BIN1, int BIN2, int PWMB);
   void activa(void);
   void fijaPWMs(int pwmI, int pwmD);
   void fijaDirs(int dir_izq, int dir_der);
   void desconecta(void);
   void frena(void);
   void frenaMotIzq(void);
   void frenaMotDer(void);
};