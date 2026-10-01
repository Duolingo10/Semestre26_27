#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "interfaz.h"

void inicializar(struct Registro *reg) {
    reg->ID = 1;
    reg->PC = 1;
    reg->AX = 0;
    reg->BX = 0;
    reg->CX = 0;
    reg->DX = 0;
    strcpy(reg->IR, "");
    strcpy(reg->Status, "");
}

void imprimirRegistro(const struct Registro *reg) {
    printf("| %-2d | %-2d | %-4d | %-4d | %-4d | %-4d | %-15s | %-22s |\n",
           reg->ID, reg->PC, reg->AX, reg->BX, reg->CX, reg->DX, reg->IR, reg->Status);
}

int esNumeroValido(const char *str) {
    if (!str || *str == '\0') return 0;
    int i = 0;
    if (str[0] == '-') {
        if (strlen(str) == 1) return 0;
        i = 1;
    }
    for (; str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)str[i])) return 0;
    }
    return 1;
}

int *obtenerRegistro(struct Registro *reg, const char *nombre) {
    if (!nombre) return NULL;
    if (strcmp(nombre, "AX") == 0) return &reg->AX;
    if (strcmp(nombre, "BX") == 0) return &reg->BX;
    if (strcmp(nombre, "CX") == 0) return &reg->CX;
    if (strcmp(nombre, "DX") == 0) return &reg->DX;
    return NULL;
}

void procesarLinea(struct Registro *reg, char *linea) {
    char copia[100];
    strcpy(copia, linea);
    strcpy(reg->IR, linea);
    strcpy(reg->Status, "");

    // Refactorización: Reemplazo de accesos fijos por strtok()
    char *instruccion = strtok(copia, " ,\t\r\n");
    if (!instruccion) return;

    int esUnaria = (strcmp(instruccion, "INC") == 0 || strcmp(instruccion, "DEC") == 0);
    int esBinaria = (strcmp(instruccion, "MOV") == 0 || strcmp(instruccion, "ADD") == 0 ||
                     strcmp(instruccion, "SUB") == 0 || strcmp(instruccion, "MUL") == 0 ||
                     strcmp(instruccion, "MULT") == 0 || strcmp(instruccion, "DIV") == 0);

    if (!esUnaria && !esBinaria) {
        strcpy(reg->Status, "ERROR de sintaxis");
        return;
    }

    char *destStr = strtok(NULL, " ,\t\r\n");
    if (!destStr) {
        strcpy(reg->Status, "ERROR de sintaxis");
        return;
    }

    int *regDestino = obtenerRegistro(reg, destStr);
    if (!regDestino) {
        strcpy(reg->Status, "ERROR, registro invalido");
        return;
    }

    if (esUnaria) {
        if (strtok(NULL, " ,\t\r\n") != NULL) {
            strcpy(reg->Status, "ERROR de sintaxis");
            return;
        }
        if (strcmp(instruccion, "INC") == 0) (*regDestino)++;
        else if (strcmp(instruccion, "DEC") == 0) (*regDestino)--;
    } 
    else if (esBinaria) {
        char *origenStr = strtok(NULL, " ,\t\r\n");
        if (!origenStr || strtok(NULL, " ,\t\r\n") != NULL) {
            strcpy(reg->Status, "ERROR de sintaxis");
            return;
        }

        int valorOrigen = 0;
        int *regOrigen = obtenerRegistro(reg, origenStr);

        if (regOrigen) {
            valorOrigen = *regOrigen;
        } else if (esNumeroValido(origenStr)) {
            valorOrigen = atoi(origenStr);
        } else {
            strcpy(reg->Status, "ERROR, operando inválido");
            return;
        }

        // Manejo correcto de error de división por cero
        if (strcmp(instruccion, "DIV") == 0 && valorOrigen == 0) {
            strcpy(reg->Status, "ERROR, División por cero");
            return;
        }

        if (strcmp(instruccion, "MOV") == 0) *regDestino = valorOrigen;
        else if (strcmp(instruccion, "ADD") == 0) *regDestino += valorOrigen;
        else if (strcmp(instruccion, "SUB") == 0) *regDestino -= valorOrigen;
        else if (strcmp(instruccion, "MUL") == 0 || strcmp(instruccion, "MULT") == 0) *regDestino *= valorOrigen;
        else if (strcmp(instruccion, "DIV") == 0) *regDestino /= valorOrigen;
    }
}

void ejecutarArchivo(struct Registro *reg, const char *nombreArchivo) {
    FILE *archivo = fopen(nombreArchivo, "r");
    if (!archivo) {
        printf("ERROR proceso no encontrado\n");
        return;
    }

    char linea[100];
    printf("| ID | PC | AX   | BX   | CX   | DX   | IR              | Status                 |\n");
    printf("----------------------------------------------------------------------------------\n");

    while (fgets(linea, sizeof(linea), archivo)) {
        linea[strcspn(linea, "\r\n")] = '\0';
        if (strlen(linea) == 0) continue;

        procesarLinea(reg, linea);
        imprimirRegistro(reg);
        reg->PC++;
    }
    fclose(archivo);
}