#include "10Estados.h"

char * nombreEstado[]={"LISTO", "EJECUCION", "BLOQUEADO","FINALIZADO"};

char * nombreTransicion[]={ "DESPERTAR","DESPACHAR", "BLOQUEAR",  "FINALIZAR"};


int cambiarEstado(int estadoActual, int eventoOcurrido) {
  
  int nuevoEstado=estadoActual;

  if(estadoActual==eventoOcurrido){
    return -1;
  }

  if(eventoOcurrido==FINALIZAR){
    nuevoEstado=FINALIZADO;
  }
  
  switch (estadoActual) {
    
    case LISTO: if (eventoOcurrido==DESPACHAR) nuevoEstado=EJECUCION;
      break;
      
    case EJECUCION: if (eventoOcurrido==FINALIZAR) nuevoEstado=FINALIZADO;
      else
	if (eventoOcurrido==BLOQUEAR) nuevoEstado=BLOQUEADO;
      break;
      
    case BLOQUEADO: if (eventoOcurrido==DESPERTAR) nuevoEstado=LISTO;
      break;
  }
  
  return(nuevoEstado);
}

char * getNombreEstado(int estado) {
  
      return nombreEstado[estado];
}


char * getNombreTransicion(int transicion) {
  
      return nombreTransicion[transicion];
}
