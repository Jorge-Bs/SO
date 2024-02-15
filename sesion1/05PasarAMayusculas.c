#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CADENA_POR_DEFECTO "Fernando Alvarez Garcia"

#define POSICION_CADENA_EN_LINEA_COMANDOS 1

int estaEn(char caracter,char lista[]);

void main (int argc, char *argv[]) {
  
  char cadena[]=CADENA_POR_DEFECTO;
  char alfabeto[]={'a','b','c','d','e','f','g','h','i','j'};

  int len = sizeof(cadena)/sizeof(char);
  
  if (argc==2)
      strcpy(cadena, argv[POSICION_CADENA_EN_LINEA_COMANDOS]);
  
  /**
  char cadenaCorta[11];

  for(int i=0;i<10;i++){
    cadenaCorta[i]=cadena[i];
    //printf("C1: %c, C: %c",cadenaCorta[i],cadena[i]);
  }**/

  //cadenaCorta[11]='\0';

  for(int i=0;i<len;i++){
    int value = estaEn(cadena[i],alfabeto);
    if(value==1){
      cadena[i]=toupper(cadena[i]);
    }
  }

  //printf(cadenaCorta,"\n");

  //pasarAMayusculas(cadenaCorta);
  printf("La cadena de caracteres es ahora %s\n",cadena);
  //printf("La cadena de caracteres es ahora %s\n",cadena);
  
  exit(1);
}

int estaEn(char caracter,char lista[]){
    int len = sizeof(lista)/sizeof(char);

    for( int i=0;i<len;i++){
      if(lista[i]==caracter){
        return 1;
      }
    }
    return 0;
}

void pasarAMayusculas(char cadena[]) {
  
  int i;
  int longitud=strlen(cadena);
  
  for (i=0; i<longitud;i++)
    cadena[i]=toupper(cadena[i]);
}


// 1. Compilar con "gcc 05PasarAMayusculas.c"
// 2. Ejecutar con "./a.out"
// 3. Obtener la ayuda sobre strcpy, strlen, toupper: "man strcpy", "man strlen", "man toupper"
// 4. Modificar el programa para que sólo pase a mayúsculas las 10 primeras letras del alfabeto