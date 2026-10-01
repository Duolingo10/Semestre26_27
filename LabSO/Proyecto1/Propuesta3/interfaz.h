#ifndef INTERFAZ_H
#define INTERFAZ_H

struct Registro {
    int ID;
    int PC;
    int AX, BX, CX, DX;
    char IR[100];
    char Status[50];
};

void inicializar(struct Registro *reg);
void imprimirRegistro(const struct Registro *reg);
int esNumeroValido(const char *str);
int *obtenerRegistro(struct Registro *reg, const char *nombre);
void procesarLinea(struct Registro *reg, char *linea);
void ejecutarArchivo(struct Registro *reg, const char *nombreArchivo);

#endif