#ifndef INTERFAZ_H
#define INTERFAZ_H


struct Reguistro {
    int ID;
    int PC;
    int AX;
    int BX;
    int CX;
    int DX;
};

struct Reguistro *crearReguistro(int valor);
int indentificadorOperador(const char *linea);
int indentificadorBase(const char *linea);
int indentificadorModificador(const char *linea);
void analisisadorComando(char *linea, struct Reguistro **objeto);
void lecturaArchivo(struct Reguistro **objeto);

#endif
