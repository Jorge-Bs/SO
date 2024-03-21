// V1
#include "Clock.h"
#include "Processor.h"
# include "ComputerSystemBase.h" // Ejercicio V2 - d

int tics=0;

void Clock_Update() {
	tics++;
	//Inicio V2-Ej1-D
	if((tics%intervalBetweenInterrupts)==0){
		Processor_RaiseInterrupt(CLOCKINT_BIT);
	}
	//Fin V2-Ej1-D
}


int Clock_GetTime() {

	return tics;
}
