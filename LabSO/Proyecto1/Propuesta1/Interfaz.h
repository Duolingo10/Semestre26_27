#ifndef INTERFAZ_H
#define INTERFAZ_H

// Mensajes de error exactos del sistema
#define ERR_CMD "ERROR, comando invalido"
#define ERR_NO_FILE "ERROR proceso no encontrado"
#define ERR_SINTAXIS "ERROR de sintaxis"
#define ERR_REG "ERROR, registro invalido"
#define ERR_OPER "ERROR, operando inválido"
#define ERR_DIV0 "ERROR, División por cero"

// Estructura principal del CPU[cite: 3]
typedef struct {
    int ID;
    int PC;
    int AX;
    int BX;
    int CX;
    int DX;
    char IR[100];
    char Status[50];
} CPU;

// Declaración de funciones
void inicializarCPU(CPU *cpu, int id);
void imprimirTabla(const CPU *cpu);
int esNumeroValido(const char *str);
int obtenerIndiceRegistro(const char *str);
void ejecutarInstruccion(CPU *cpu, char *lineaOriginal);
void procesarArchivo(CPU *cpu, const char *nombreArchivo);

#endif