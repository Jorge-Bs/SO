Blanco Sánchez, Jorge

Se han completado los 15 ejercicios

Programas De prueba:

	prog-V1-E3---> programa que se ejecuta dos veces cuando se llama al simulador: ./Simulator prog-V1-E3 prog-V1-E3

	prog-V1-E6---> programa que solicita más espacio de memoria de la que existe en el sistema

	prog-V1-E7---> programa que tiene más instrucciones que su tamaño

	prog-V1-E12---> programa que posee la misma prioridad, se llama al simulador con dos veces este programa, y se comprueba el funcionamiento de la instrucción trap 4

	prog-V1-E13---> programa que se utiliza para comprobar que se guardan correctamente los datos

	prog-V1-E15---> programa que contiene instrucciones os, halt e iret para comprobar que se lanzan excepciones

Preguntas:

	Ejercicio 13

a ->    se necesita guardar para saber que instruccion se iba a ejecutar en el momento del la interrupción
b ->    Si, los registros a y b, y el registro acumulador
c ->    si hay que modificarla, ya que necesitamos los valores que se han guardado previamente
d ->    se necesitaria modicar los campos del pcb para que se puedan guardar dichos valores

	Ejercicio 15
	
	El modo protegido se activa cuando se da una excepción, la cpu al recibirla activa este modo y llama al sistema operativo, una vez que sea procesada la expeción, el sistema operativo desactiva este modo y cede el control de la cpu al siguiente programa de la lista de listos