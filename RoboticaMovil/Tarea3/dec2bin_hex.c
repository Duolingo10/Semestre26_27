#include <stdio.h> 
#include <stdlib.h> 

int main(int argc, char *argv[]) { 
    int numero = atoi(argv[1]);    // Convierte el argumento recibido en entero
    
    if (numero == 0) {             
        printf("equivalente en binario: 0\n"); 
    } else {                       // Si el numero es mayor a cero
        int binario[32];           // Guardamos cada bit del resultado
        int i = 0;                 // Contador para controlar las posiciones del arreglo
        int temp = numero;         // Variable temporal para hacer las divisiones
        
        while (temp > 0) {         // Ciclo que se repite mientras el numero sea mayor a cero
            binario[i] = temp % 2; // Guarda el residuo de la division entre 2 (un 0 o un 1)
            temp = temp / 2;       // Divide el numero entre 2 para continuar el proceso
            i++;                   // Incrementa la posicion en el arreglo
        }                         
        
        printf("equivalente en binario: "); 
        for (int j = i - 1; j >= 0; j--) {  // Lee el arreglo desde el final hasta el inicio
            printf("%d", binario[j]);      // Imprime cada uno de los bits
        }                                 
        printf("\n");                     
    }                              
    
    printf("equivalente en hexadecimal: %X\n", numero); // Imprime el numero a Hexadecimal
    
    return 0;                      
}