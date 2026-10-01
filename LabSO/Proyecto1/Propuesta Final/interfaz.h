#ifndef INTERFAZ_H
#define INTERFAZ_H
#include <ncurses.h>

struct Reguistro {
    int ID;
    int PC;
    int AX;
    int BX;
    int CX;
    int DX;
};

struct Reguistro *crearReguistro(int valor);
int indentificadorNumeros(const char *str);
int esFinDeLineaLimpio(const char *str);
int indentificadorOperador(const char *linea);
int* indentificadorBase(const char *linea, struct Reguistro *objeto);
//int indentificadorBase(const char *linea);
int indentificadorModificador(const char *linea, struct Reguistro *objeto);
int analisisadorComando(char *linea, struct Reguistro **objeto);
void lecturaArchivo(struct Reguistro **objeto, const char *nomArchivo);

#endif
