#include <stdio.h>
#include <stdlib.h>

main (int argc, char *argv[]) {

  int i;
  
  printf("Número de cadenas en línea de comandos es: %d\n", argc);
  
  printf("La línea de comandos completa es: \"");
  for (i=1; i<argc;i+=2)
    printf("%s ",argv[i]);
  printf("\"\n");
  exit(1);
}

// 1. Compilar con "gcc 03LineaComandos.c"
// 2. Ejecutar con "./a.out"
// 3. Modificar el programa para que muestre únicamente las cadenas que ocupen posiciones impares en línea de comandos