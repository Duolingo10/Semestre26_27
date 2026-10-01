#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include "operaciones.h"
#include "interfaz.h"

void menu(char *buffer, size_t tamano) {
    clear();
    mvprintw(1, 2,"Ingrese comando (ejecutar <archivo> / salir): ");
    refresh();
    
    echo(); 
    getnstr(buffer, tamano - 1); 
    noecho(); 
}

int esArchivoASM(const char *nombreArchivo) {
    if (nombreArchivo == NULL) return 0;
    size_t longitud = strlen(nombreArchivo);
    if (longitud < 5) return 0;
    return (strcmp(nombreArchivo + longitud - 4, ".asm") == 0);
}

int indentificadorComando(const char *linea) {
    if (strcmp(linea, "ejecutar") == 0) return 1;
    if (strcmp(linea, "salir") == 0) return 2;
    return 0; 
}

int main(void){
    int valor = 0, respuesta=0, num=0;
    char bufferEntrada[100], *token, *tokens[10];
    struct Reguistro *inicio = NULL;
    inicio = crearReguistro(valor);

    initscr();
    cbreak();             
    keypad(stdscr, TRUE); 
    curs_set(1);
    
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
                if (num == 2 && esArchivoASM(tokens[1])) {
                    inicio->AX=0; inicio->BX=0; inicio->CX=0;
                    inicio->DX=0; inicio->ID++; inicio->PC=0;
               
                    lecturaArchivo(&inicio, tokens[1]);
                } else {
                    mvprintw(5, 2,"Error: Sintaxis incorrecta. Uso: ejecutar <nombre_archivo>");
                    mvprintw(6, 2, "Presione cualquier tecla para continuar...");
                    refresh();
                    getch();
                }
                break;
            case 2:
                mvprintw(5, 2, "Cerrando programa...");
                refresh();
                napms(1000);
                break;            
            default:
                mvprintw(5, 2, "Opcion no valida.");
                mvprintw(6, 2, "Presione cualquier tecla para continuar...");
                refresh();
                getch();
                break;
            }
        }
        num=0;
    }while(respuesta!=2);
    free(inicio);
    endwin();
    return 0;
}