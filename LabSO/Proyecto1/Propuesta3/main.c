#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interfaz.h"

int main(void) {
    char entrada[100];
    struct Registro reg;

    // Sustitución del menú scanf por el bucle CLI con prompt '>'
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
            inicializar(&reg);
            ejecutarArchivo(&reg, archivo);
        } else {
            printf("ERROR, comando invalido\n");
        }
    }
    return 0;
}