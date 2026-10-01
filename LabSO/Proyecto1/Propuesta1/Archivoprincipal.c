#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Interfaz.h"

void inicializarCPU(CPU *cpu, int id) {
    cpu->ID = id;
    cpu->PC = 1;
    cpu->AX = 0;
    cpu->BX = 0;
    cpu->CX = 0;
    cpu->DX = 0;
    strcpy(cpu->IR, "");
    strcpy(cpu->Status, "");
}

void imprimirTabla(const CPU *cpu) {
    printf("| %-2d | %-2d | %-4d | %-4d | %-4d | %-4d | %-15s | %-22s |\n",
           cpu->ID, cpu->PC, cpu->AX, cpu->BX, cpu->CX, cpu->DX, cpu->IR, cpu->Status);
}

// Valida si una cadena representa un número entero válido (soporta números negativos)
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

// Retorna el índice de registro (0=AX, 1=BX, 2=CX, 3=DX) o -1 si no es válido
int obtenerIndiceRegistro(const char *str) {
    if (!str) return -1;
    if (strcmp(str, "AX") == 0) return 0;
    if (strcmp(str, "BX") == 0) return 1;
    if (strcmp(str, "CX") == 0) return 2;
    if (strcmp(str, "DX") == 0) return 3;
    return -1;
}

void ejecutarInstruccion(CPU *cpu, char *lineaOriginal) {
    char copia[100];
    strcpy(copia, lineaOriginal);
    strcpy(cpu->IR, lineaOriginal);
    strcpy(cpu->Status, "");

    // Extraer la operación ignorando espacios, comas y saltos de línea[cite: 3]
    char *op = strtok(copia, " ,\t\r\n");
    if (!op) return;

    int esUnaria = (strcmp(op, "INC") == 0 || strcmp(op, "DEC") == 0);
    int esBinaria = (strcmp(op, "MOV") == 0 || strcmp(op, "ADD") == 0 ||
                     strcmp(op, "SUB") == 0 || strcmp(op, "MUL") == 0 ||
                     strcmp(op, "MULT") == 0 || strcmp(op, "DIV") == 0);

    if (!esUnaria && !esBinaria) {
        strcpy(cpu->Status, ERR_SINTAXIS);
        return;
    }

    // Extraer primer operando (debe ser un registro obligatorio)[cite: 3]
    char *arg1 = strtok(NULL, " ,\t\r\n");
    if (!arg1) {
        strcpy(cpu->Status, ERR_SINTAXIS);
        return;
    }

    int idx1 = obtenerIndiceRegistro(arg1);
    if (idx1 == -1) {
        strcpy(cpu->Status, ERR_REG);
        return;
    }

    int *regs[4] = {&cpu->AX, &cpu->BX, &cpu->CX, &cpu->DX};

    if (esUnaria) {
        // Para INC y DEC, verificar que no existan argumentos adicionales[cite: 1, 3]
        if (strtok(NULL, " ,\t\r\n") != NULL) {
            strcpy(cpu->Status, ERR_SINTAXIS);
            return;
        }

        if (strcmp(op, "INC") == 0) (*regs[idx1])++;
        else if (strcmp(op, "DEC") == 0) (*regs[idx1])--;
    } else if (esBinaria) {
        char *arg2 = strtok(NULL, " ,\t\r\n");
        if (!arg2) {
            strcpy(cpu->Status, ERR_SINTAXIS);
            return;
        }

        // Verificar que no existan más de 2 operandos[cite: 3]
        if (strtok(NULL, " ,\t\r\n") != NULL) {
            strcpy(cpu->Status, ERR_SINTAXIS);
            return;
        }

        int val2 = 0;
        int idx2 = obtenerIndiceRegistro(arg2);
        if (idx2 != -1) {
            val2 = *regs[idx2];
        } else if (esNumeroValido(arg2)) {
            val2 = atoi(arg2);
        } else {
            strcpy(cpu->Status, ERR_OPER);
            return;
        }

        // Manejo de división entre cero[cite: 1, 3]
        if (strcmp(op, "DIV") == 0 && val2 == 0) {
            strcpy(cpu->Status, ERR_DIV0);
            return;
        }

        if (strcmp(op, "MOV") == 0) *regs[idx1] = val2;
        else if (strcmp(op, "ADD") == 0) *regs[idx1] += val2;
        else if (strcmp(op, "SUB") == 0) *regs[idx1] -= val2;
        else if (strcmp(op, "MUL") == 0 || strcmp(op, "MULT") == 0) *regs[idx1] *= val2;
        else if (strcmp(op, "DIV") == 0) *regs[idx1] /= val2;
    }
}

void procesarArchivo(CPU *cpu, const char *nombreArchivo) {
    FILE *archivo = fopen(nombreArchivo, "r");
    if (!archivo) {
        printf("%s\n", ERR_NO_FILE);
        return;
    }

    char linea[100];
    printf("| ID | PC | AX   | BX   | CX   | DX   | IR              | Status                 |\n");
    printf("----------------------------------------------------------------------------------\n");

    while (fgets(linea, sizeof(linea), archivo)) {
        linea[strcspn(linea, "\r\n")] = '\0';
        if (strlen(linea) == 0) continue;

        ejecutarInstruccion(cpu, linea);
        imprimirTabla(cpu);
        cpu->PC++;
    }
    fclose(archivo);
}

int main(void) {
    char entrada[100];
    CPU cpu;

    while (1) {
        printf(">");
        if (!fgets(entrada, sizeof(entrada), stdin)) break;
        entrada[strcspn(entrada, "\r\n")] = '\0';

        if (strlen(entrada) == 0) continue;

        if (strcmp(entrada, "SALIR") == 0) {
            break;
        }

        char copia[100];
        strcpy(copia, entrada);
        char *cmd = strtok(copia, " ");
        char *archivo = strtok(NULL, " ");
        char *extra = strtok(NULL, " ");

        if (cmd && strcmp(cmd, "EJECUTAR") == 0 && archivo && !extra) {
            inicializarCPU(&cpu, 1);
            procesarArchivo(&cpu, archivo);
        } else {
            printf("%s\n", ERR_CMD);
        }
    }
    return 0;
}