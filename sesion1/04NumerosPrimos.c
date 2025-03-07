#include <stdio.h>
#include <stdlib.h>

main (int argc, char *argv[]) {

  int numerosPrimos[]={1,2,3,5,7,11,13,17,19,23,29};
  int posicion;
  int suma=0;

  if (argc==1)
    printf("La lista contiene los %d primeros números primos\n",sizeof(numerosPrimos)/sizeof(int));
  else {
    posicion=atoi(argv[1]);
    printf("El número primo que ocupa la posición %d es %d\n",posicion, numerosPrimos[posicion]);
  }
  for(int i=0;i<11;i++){
    if(i!=posicion){
      suma+=numerosPrimos[i];
    }
  }
  printf("Las suma de los numero primos menos es %d, es de: %d",numerosPrimos[posicion],suma);
  exit(1);
}


// 1. Compilar con "gcc 04NumerosPrimos.c"
// 2. Ejecutar con "./a.out"
// 3. Obtener la ayuda sobre atoi: "man atoi"
// 4. Modificar el programa para que calcule y muestre la suma de todos los números primos de la lista excepto el indicado en línea de comandos