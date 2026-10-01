#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "operacioness.h"
#include "interfazz.h"

void menu(char *buffer, size_t tamano) {
    printf("Ingrese comando (Ejecuta <archivo> / Salir): ");
    if (fgets(buffer, tamano, stdin) != NULL) {
        buffer[strcspn(buffer, "\r\n")] = '\0'; // Limpia el salto de línea
    } else {
        buffer[0] = '\0';
    }
}

int indentificadorComando(const char *linea) {
    if (strcmp(linea, "Ejecuta") == 0) return 1;
    if (strcmp(linea, "Salir") == 0) return 2;
    return 0; 
}

int main(void){
    int valor = 0, respuesta=0, num=0;
    char bufferEntrada[100], *token, *tokens[10];
    struct Reguistro *inicio = NULL;
    inicio = crearReguistro(valor);

    do{
        menu(bufferEntrada, sizeof(bufferEntrada));
        
        token = strtok(bufferEntrada, " ");
        while (token != NULL && num < 10) {
            tokens[num] = token;
            token = strtok(NULL, " ");
            num++;
        }
        if(num>0){
            respuesta=indentificadorComando(tokens[0]);
            switch (respuesta)
            {
            case 1:
                if (num >= 2) {
                    lecturaArchivo(&inicio, tokens[1]);
                } else {
                    printf("Error: Sintaxis incorrecta. Uso: Ejecuta <nombre_archivo>\n");
                }
                printf("____________________\n");
                break;

            case 2:
                printf("\nCerrando progarma.");
                printf("____________________\n");
                break;

            
            default:
                printf("\nOpcion no valida");
                printf("\n____________________\n");
                break;
            }
        }
        num=0;
    }while(respuesta!=2);
    free(inicio);
    return 0;
}