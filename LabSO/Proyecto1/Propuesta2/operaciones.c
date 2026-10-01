#include "operacioness.h"

int ADD(int base, int modificador) { return base + modificador; }
int SUB(int base, int modificador) { return base - modificador; }
int MOV(int base, int modificador) { (void)base; return modificador; }
int MUL(int base, int modificador) { return base * modificador; }
int DIV(int base, int modificador) { return base / modificador; }
int INC(int base) { return base + 1; }
int DEC(int base) { return base - 1; }