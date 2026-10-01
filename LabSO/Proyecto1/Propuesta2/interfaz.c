#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "interfazz.h"
#include "operacioness.h"

struct Reguistro *crearReguistro(int valor) {
    struct Reguistro *nodo = malloc(sizeof(struct Reguistro));
    if (nodo) {
        nodo->ID = valor + 1;
        nodo->PC = 1;
        nodo->AX = 0; nodo->BX = 0; nodo->CX = 0; nodo->DX = 0;
    }
    return nodo;
}

int indentificadorNumeros(const char *str) {
    if (str == NULL || *str == '\0') return 0; 
    if (*str == '-') str++;
    if (*str == '\0') return 0;
    while (*str) {
        if (!isdigit((unsigned char)*str)) return 0;
        str++;
    }
    return 1; 
}

int indentificadorModificador(const char *linea, struct Reguistro *objeto) {
    int valor;
    if (indentificadorNumeros(linea)) {
        valor = atoi(linea);
        if (valor > 99998 || valor < -99998) valor = 99999; 
    } else {
        if (strcmp(linea, "AX") == 0) valor = objeto->AX;
        else if (strcmp(linea, "BX") == 0) valor = objeto->BX;
        else if (strcmp(linea, "CX") == 0) valor = objeto->CX;
        else if (strcmp(linea, "DX") == 0) valor = objeto->DX;
        else valor = 99999;
    }
    return valor;
}

int indentificadorOperador(const char *linea) {
    if (strcmp(linea, "ADD") == 0) return 1;
    if (strcmp(linea, "SUB") == 0) return 2;
    if (strcmp(linea, "MOV") == 0) return 3;
    if (strcmp(linea, "MUL") == 0) return 4;
    if (strcmp(linea, "DIV") == 0) return 5;
    if (strcmp(linea, "DEC") == 0) return 6;
    if (strcmp(linea, "INC") == 0) return 7;
    return 0; 
}

int* indentificadorBase(const char *linea, struct Reguistro *objeto) {
    if (strcmp(linea, "AX") == 0) return &(objeto->AX);
    if (strcmp(linea, "BX") == 0) return &(objeto->BX);
    if (strcmp(linea, "CX") == 0) return &(objeto->CX);
    if (strcmp(linea, "DX") == 0) return &(objeto->DX);    
    return NULL;
}

int analisisadorComando(char *linea, struct Reguistro **objeto) {
    int Error = 0;
    if (linea != NULL && objeto != NULL && *objeto != NULL) {
        int nTokens = 0, nSubTokens = 0, R = 0, operador = 0, *base = NULL, modificador = 0;
        char *saveptr1, *saveptr2;
        char *tokens[10], *token, *subtokens[10], *subtoken;
        char *txC = malloc(strlen(linea) + 1); 
        if (txC == NULL) return 1; 
            
        strcpy(txC, linea);
        token = strtok_r(txC, " ", &saveptr1);
        while (token != NULL && nTokens < 10) {
            tokens[nTokens++] = token;
            token = strtok_r(NULL, " ", &saveptr1);
        }
        
        if (nTokens == 2) {
            operador = indentificadorOperador(tokens[0]);
            if (operador >= 1 && operador <= 5) {
                subtoken = strtok_r(tokens[1], ",", &saveptr2);
                while (subtoken != NULL && nSubTokens < 10) {
                    subtokens[nSubTokens++] = subtoken;
                    subtoken = strtok_r(NULL, ",", &saveptr2);
                }
                if (nSubTokens == 2) {
                    base = indentificadorBase(subtokens[0], *objeto);
                    modificador = indentificadorModificador(subtokens[1], *objeto);
                    if (base != NULL) {
                        switch (operador) {
                            case 1: R = *base + modificador; break;
                            case 2: R = *base - modificador; break;
                            case 3: R = modificador; break;
                            case 4: R = *base * modificador; break;
                            case 5: 
                                if (modificador != 0) R = *base / modificador;
                                else Error = 1;
                                break;
                        }
                    }
                    if (Error == 1 || base == NULL || modificador == 99999 || R >= 99999 || R <= -99999) Error = 1;
                } else Error = 1;
            } else if (operador == 6 || operador == 7) {
                base = indentificadorBase(tokens[1], *objeto);
                if (base != NULL) {
                    switch (operador) {
                        case 6: R = *base - 1; break;
                        case 7: R = *base + 1; break;
                    }
                }
                if (base == NULL || R >= 99999 || R <= -99999) Error = 1;
            } else Error = 1;
        } else Error = 1;

        if (Error == 0) {
            switch (operador) {
                case 1: *base = ADD(*base, modificador); break;
                case 2: *base = SUB(*base, modificador); break;
                case 3: *base = MOV(*base, modificador); break;
                case 4: *base = MUL(*base, modificador); break;
                case 5: *base = DIV(*base, modificador); break;
                case 6: *base = DEC(*base); break;
                case 7: *base = INC(*base); break;
            }
        }
        free(txC);
    } else Error = 1;

    return Error;
}

void lecturaArchivo(struct Reguistro **objeto, const char *nomArchivo) {
    FILE *archivo = fopen(nomArchivo, "r");
    char linea[100], *aviso;
    int Error;

    if (archivo != NULL) {
        printf("| ID   | PC   | AX   | BX   | CX   | DX   | Comando         | Estado           |\n");
        while (fgets(linea, sizeof(linea), archivo) != NULL) {
            linea[strcspn(linea, "\r\n")] = '\0';
            if (strlen(linea) > 0) {
                Error = analisisadorComando(linea, objeto);
                aviso = (Error == 0) ? "Ok" : "Error de sintaxis";
                
                printf("|%-6d|%-6d|%-6d|%-6d|%-6d|%-6d|%-17s|%-18s|\n",
                       (*objeto)->ID, (*objeto)->PC, (*objeto)->AX, 
                       (*objeto)->BX, (*objeto)->CX, (*objeto)->DX, linea, aviso);
                
                (*objeto)->PC++; // Incremento correcto de PC
            }
        }
        fclose(archivo);
    } else {
        printf("Error al abrir el archivo\n");
    }
}