#include <stdio.h> 

int main(int argc, char *argv[]) { // Punto de inicio del programa con argumentos de consola
    int numero_decimal;            // Variable para almacenar el resultado en base decimal
    
    sscanf(argv[1], "%x", &numero_decimal); // Lee el texto hexadecimal (%x) y lo convierte a entero
    
    printf("equivalente en decimal: %d\n", numero_decimal); 
    
    return 0;                      
}