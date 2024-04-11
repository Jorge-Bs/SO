Blanco Sánchez, Jorge

Se han completado los 6 ejercicios

Programas De prueba(V1):

	prog-V1-E3---> programa que se ejecuta dos veces cuando se llama al simulador: ./Simulator prog-V1-E3 prog-V1-E3

	prog-V1-E6---> programa que solicita más espacio de memoria de la que existe en el sistema

	prog-V1-E7---> programa que tiene más instrucciones que su tamaño

	prog-V1-E12---> programa que posee la misma prioridad, se llama al simulador con dos veces este programa, y se comprueba el funcionamiento de la instrucción trap 4

	prog-V1-E13---> programa que se utiliza para comprobar que se guardan correctamente los datos

	prog-V1-E15---> programa que contiene instrucciones os, halt e iret para comprobar que se lanzan excepciones

Programas De prueba(V2):

	prog-V2-E5---> programa para comprobar que un proceso se duerme correctamente
	prog-V2-61---> programa que intenta ceder el procesador, se duerme y acaba
	prog-V2-62---> realiza un bucle, intenta ceder el control, se duerme dos veces, vuelve a intentar ceder el control y acaba
	prog-V2-63---> programa parecido al prog-V2-61, sus instrucciones de trap son dobles

	El conjunto de estos 3 procesos se ha creado para comprobar el correcto funcionamiento del ejercicio 6 y comprobar si se duerme y se despiertan, y cediendo la cpu cuando sea necesario, los procesos correctamente

Para comprobar que se cede correctamente un proceso, ahora no presente en el código, en OperatingSystem_CheckPriority() y en el if(processTable[executingProcessID].priority>readyPriority), lo comprobé con un >= con los programas anteriores
	