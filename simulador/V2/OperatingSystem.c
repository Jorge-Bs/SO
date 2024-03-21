#include "Simulator.h"
#include "OperatingSystem.h"
#include "OperatingSystemBase.h"
#include "MMU.h"
#include "Processor.h"
#include "Buses.h"
#include "Heap.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

// Functions prototypes
void OperatingSystem_PCBInitialization(int, int, int, int, int);
void OperatingSystem_MoveToTheREADYState(int);
void OperatingSystem_Dispatch(int);
void OperatingSystem_RestoreContext(int);
void OperatingSystem_SaveContext(int);
void OperatingSystem_TerminateExecutingProcess();
int OperatingSystem_LongTermScheduler();
void OperatingSystem_PreemptRunningProcess();
int OperatingSystem_CreateProcess(int);
int OperatingSystem_ObtainMainMemory(int, int);
int OperatingSystem_ShortTermScheduler();
int OperatingSystem_ExtractFromReadyToRun();
void OperatingSystem_HandleException();
void OperatingSystem_HandleSystemCall();


//Inicio V1-Ej9
void  OperatingSystem_PrintReadyToRunQueue();
//Final V1-Ej9

// The process table
// PCB processTable[PROCESSTABLEMAXSIZE];
PCB * processTable;

// Size of the memory occupied for the OS
int OS_MEMORY_SIZE=32;

// Address base for OS code in this version
int OS_address_base; 

// Identifier of the current executing process
int executingProcessID=NOPROCESS;

// Identifier of the System Idle Process
int sipID;

// Initial PID for assignation (Not assigned)
int initialPID=-1;

// Begin indes for daemons in programList
// int baseDaemonsInProgramList; 

// Array that contains the identifiers of the READY processes
// heapItem readyToRunQueue[NUMBEROFQUEUES][PROCESSTABLEMAXSIZE];

heapItem *readyToRunQueue[NUMBEROFQUEUES];
//int numberOfReadyToRunProcesses[NUMBEROFQUEUES]={0};
//Incicio V1-Ej11-A
int numberOfReadyToRunProcesses[NUMBEROFQUEUES]={0,0}; 
char * queueNames [NUMBEROFQUEUES]={"USER","DAEMONS"}; 
//Fin V1-Ej11-A

// Variable containing the number of not terminated user processes
int numberOfNotTerminatedUserProcesses=0;

// char DAEMONS_PROGRAMS_FILE[MAXFILENAMELENGTH]="teachersDaemons";

int MAINMEMORYSECTIONSIZE = 60;

extern int MAINMEMORYSIZE;

int PROCESSTABLEMAXSIZE = 4;

//Inicio V1-EJ10-A
char * statesNames [5]={"NEW","READY","EXECUTING","BLOCKED","EXIT"};
//Fin V1-EJ10-A


// Initial set of tasks of the OS
void OperatingSystem_Initialize(int programsFromFileIndex) {
	
	int i, selectedProcess;
	FILE *programFile; // For load Operating System Code
	
// In this version, with original configuration of memory size (300) and number of processes (4)
// every process occupies a 60 positions main memory chunk 
// and OS code and the system stack occupies 60 positions 

	OS_address_base = MAINMEMORYSIZE - OS_MEMORY_SIZE;

	MAINMEMORYSECTIONSIZE = OS_address_base / PROCESSTABLEMAXSIZE;

	//Inicio V1-EJ8
	if (initialPID<0) // if not assigned in options...
		initialPID=PROCESSTABLEMAXSIZE-1; 
	
	//Fin V1-Ej8
	// Space for the processTable
	processTable = (PCB *) malloc(PROCESSTABLEMAXSIZE*sizeof(PCB));
	
	// Space for the ready to run queues (one queue initially...)
	//readyToRunQueue[ALLPROCESSESQUEUE] = Heap_create(PROCESSTABLEMAXSIZE);

	//Inicio V1-Ej11-B
	readyToRunQueue[USERPROCESSQUEUE] = Heap_create(PROCESSTABLEMAXSIZE);
	readyToRunQueue[DAEMONSQUEUE] = Heap_create(PROCESSTABLEMAXSIZE);
	//Fin V1-Ej11-B

	programFile=fopen("OperatingSystemCode", "r");
	if (programFile==NULL){
		// Show red message "FATAL ERROR: Missing Operating System!\n"
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,99,SHUTDOWN,"FATAL ERROR: Missing Operating System!\n");
		exit(1);		
	}

	// Obtain the memory requirements of the program
	int processSize=OperatingSystem_ObtainProgramSize(programFile);

	// Load Operating System Code
	OperatingSystem_LoadProgram(programFile, OS_address_base, processSize);
	
	// Process table initialization (all entries are free)
	for (i=0; i<PROCESSTABLEMAXSIZE;i++){
		processTable[i].busy=0;
		processTable[i].programListIndex=-1;
		processTable[i].initialPhysicalAddress=-1;
		processTable[i].processSize=-1;
		processTable[i].copyOfSPRegister=-1;
		processTable[i].state=-1;
		processTable[i].priority=-1;
		processTable[i].copyOfPCRegister=-1;
		processTable[i].copyOfPSWRegister=0;
		processTable[i].programListIndex=-1;
		//Inicio V1-EJ13
		processTable[i].copyARegister=0;
		processTable[i].copyBRegister=0;
		//processTable[i].copyControl=-1;
		processTable[i].copyAccumulator=0;
		//Inicio V1-EJ13
	}
	// Initialization of the interrupt vector table of the processor
	Processor_InitializeInterruptVectorTable(OS_address_base+2);
		
	// Include in program list all user or system daemon processes
	OperatingSystem_PrepareDaemons(programsFromFileIndex);
	
	// Create all user processes from the information given in the command line
	OperatingSystem_LongTermScheduler();
	
	if (strcmp(programList[processTable[sipID].programListIndex]->executableName,"SystemIdleProcess")
		&& processTable[sipID].state==READY) {
		// Show red message "FATAL ERROR: Missing SIP program!\n"
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,99,SHUTDOWN,"FATAL ERROR: Missing SIP program!\n");
		exit(1);		
	}

	//Inicio V1-Ej14// numberOfNotTerminatedUserProcesses==0 // programList[processTable[selectedProcess].programListIndex]->type==DAEMONPROGRAM
	if(numberOfReadyToRunProcesses[USERPROCESSQUEUE]==0){
		OperatingSystem_ReadyToShutdown();
	}
	//Fin V1-Ej14

	// At least, one process has been created
	// Select the first process that is going to use the processor
	selectedProcess=OperatingSystem_ShortTermScheduler();

	Processor_SetSSP(MAINMEMORYSIZE-1);

	// Assign the processor to the selected process
	OperatingSystem_Dispatch(selectedProcess);

	// Initial operation for Operating System
	Processor_SetPC(OS_address_base);

}

// The LTS is responsible of the admission of new processes in the system.
// Initially, it creates a process from each program specified in the 
// 			command line and daemons programs
int OperatingSystem_LongTermScheduler() {
  
	int PID, i,
		numberOfSuccessfullyCreatedProcesses=0;
	
	for (i=0; programList[i]!=NULL && i<PROGRAMSMAXNUMBER ; i++) {
		PID=OperatingSystem_CreateProcess(i);
		//Inicio V1-EJ4-B
		if(PID==NOFREEENTRY){
			ComputerSystem_DebugMessage(TIMED_MESSAGE,103,ERROR,programList[i]->executableName);
		}//Fin V1-EJ4-B
		
		//Incio V1-EJ5-B
		else if(PID==PROGRAMDOESNOTEXIST){
			ComputerSystem_DebugMessage(TIMED_MESSAGE,104,ERROR,programList[i]->executableName,"--- it does not exist ---");
		}
		//Fin V1-EJ5-B

		//Inicio V1-EJ5-C
		else if(PID==PROGRAMNOTVALID){
			ComputerSystem_DebugMessage(TIMED_MESSAGE,104,ERROR,programList[i]->executableName,"--- invalid priority or size ---");
		}
		//Fin V1-EJ5-C

		//Inicio V1-EJ6-B
		else if(PID==TOOBIGPROCESS){
			ComputerSystem_DebugMessage(TIMED_MESSAGE,105,ERROR,programList[i]->executableName);
		}
		//Fin V1-EJ6-B

		else{
			numberOfSuccessfullyCreatedProcesses++;
			if (programList[i]->type==USERPROGRAM) 
				numberOfNotTerminatedUserProcesses++;
			// Move process to the ready state
			OperatingSystem_MoveToTheREADYState(PID);
		}
	}

	// Return the number of succesfully created processes
	return numberOfSuccessfullyCreatedProcesses;
}

// This function creates a process from an executable program
int OperatingSystem_CreateProcess(int indexOfExecutableProgram) {
  
	int PID;
	int processSize;
	int loadingPhysicalAddress;
	int priority;
	FILE *programFile;
	PROGRAMS_DATA *executableProgram=programList[indexOfExecutableProgram];

	// Obtain a process ID
	PID=OperatingSystem_ObtainAnEntryInTheProcessTable();

	//INICIO V1-EJ4-A
	if(PID==NOFREEENTRY){
		return PID;
	}
	//FIN  V1-EJ4-A

	// Check if programFile exists
	programFile=fopen(executableProgram->executableName, "r");

	//Inicio V1-EJ5-B

	if(programFile==NULL){
		return PROGRAMDOESNOTEXIST;
	}

	//Fin V1-EJ5-B

	// Obtain the memory requirements of the program
	processSize=OperatingSystem_ObtainProgramSize(programFile);	

	// Obtain the priority for the process
	priority=OperatingSystem_ObtainPriority(programFile);

	//Inicio V1-Ej5-C
	if(processSize==PROGRAMNOTVALID || priority==PROGRAMNOTVALID){
		return PROGRAMNOTVALID;
	}
	//Fin V1-Ej5-C
	
	// Obtain enough memory space
 	loadingPhysicalAddress=OperatingSystem_ObtainMainMemory(processSize, PID);

	//Inicio V1-EJ6-A
	if(loadingPhysicalAddress==TOOBIGPROCESS){
		return TOOBIGPROCESS;
	}
	//Fin V1-EJ6-A

	// Load program in the allocated memory
	//Inicio V1-EJ7-A
	if(OperatingSystem_LoadProgram(programFile, loadingPhysicalAddress, processSize)==TOOBIGPROCESS){
		return TOOBIGPROCESS;
	}
	//Fin V1-EJ7-A


	// PCB initialization
	OperatingSystem_PCBInitialization(PID, loadingPhysicalAddress, processSize, priority, indexOfExecutableProgram);
	
	// Show message "Process [PID] created from program [executableName]\n"
	//ComputerSystem_DebugMessage(TIMED_MESSAGE,70,SYSPROC,PID,executableProgram->executableName);

	//Inicio V1-EJ10-B
	ComputerSystem_DebugMessage(TIMED_MESSAGE,111,SYSPROC,PID,statesNames[NEW],executableProgram->executableName);
	//Fin V1-EJ10-B
	
	return PID;
}


// Main memory is assigned in chunks. All chunks are the same size. A process
// always obtains the chunk whose position in memory is equal to the processor identifier
int OperatingSystem_ObtainMainMemory(int processSize, int PID) {

 	if (processSize>MAINMEMORYSECTIONSIZE)
		return TOOBIGPROCESS;
	
 	return PID*MAINMEMORYSECTIONSIZE;
}


// Assign initial values to all fields inside the PCB
void OperatingSystem_PCBInitialization(int PID, int initialPhysicalAddress, int processSize, int priority, int processPLIndex) {

	processTable[PID].busy=1;
	processTable[PID].initialPhysicalAddress=initialPhysicalAddress;
	processTable[PID].processSize=processSize;
	processTable[PID].copyOfSPRegister=initialPhysicalAddress+processSize;
	processTable[PID].state=NEW;
	processTable[PID].priority=priority;
	processTable[PID].programListIndex=processPLIndex;

	//Inicio V1-EJ11-C
	processTable[PID].queueID=USERPROCESSQUEUE;
	//Fin V1-Ej11-C
	// Daemons run in protected mode and MMU use real address
	if (programList[processPLIndex]->type == DAEMONPROGRAM) {
		processTable[PID].copyOfPCRegister=initialPhysicalAddress;
		processTable[PID].copyOfPSWRegister= ((unsigned int) 1) << EXECUTION_MODE_BIT;
		//Inicio V1-EJ11-C
		processTable[PID].queueID=DAEMONSQUEUE;
		//Fin V1-Ej11-C
	} 
	else {
		processTable[PID].copyOfPCRegister=0;
		processTable[PID].copyOfPSWRegister=0;
	}

}


// Move a process to the READY state: it will be inserted, depending on its priority, in
// a queue of identifiers of READY processes
void OperatingSystem_MoveToTheREADYState(int PID) {
	
	//Anterior ALLPROCESSESQUEUE Ahora processTable[PID].queueID Ejercicio V1-11-C
	if (Heap_add(PID, readyToRunQueue[processTable[PID].queueID],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[processTable[PID].queueID]))>=0) {
		int previous = processTable[PID].state;//V1-EJ10-B
		processTable[PID].state=READY;
		//Inicio V1-EJ10-B
		ComputerSystem_DebugMessage(TIMED_MESSAGE,110,SYSPROC,PID,programList[processTable[PID].programListIndex]->executableName,statesNames[previous],statesNames[READY]);
		//Fin V1-EJ9-B
	}
	//Inicio V1-EJ9-B
	OperatingSystem_PrintReadyToRunQueue();
	//Fin V1-EJ9-B
}


// The STS is responsible of deciding which process to execute when specific events occur.
// It uses processes priorities to make the decission. Given that the READY queue is ordered
// depending on processes priority, the STS just selects the process in front of the READY queue
int OperatingSystem_ShortTermScheduler() {
	
	int selectedProcess;

	selectedProcess=OperatingSystem_ExtractFromReadyToRun();
	
	return selectedProcess;
}


// Return PID of more priority process in the READY queue
int OperatingSystem_ExtractFromReadyToRun() {
  
	int selectedProcess=NOPROCESS;

	//selectedProcess=Heap_poll(readyToRunQueue[ALLPROCESSESQUEUE],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[ALLPROCESSESQUEUE]));
	//Inicio V1-Ej11-C
	selectedProcess=Heap_poll(readyToRunQueue[USERPROCESSQUEUE],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[USERPROCESSQUEUE]));
	if(selectedProcess==NOPROCESS){
		selectedProcess=Heap_poll(readyToRunQueue[DAEMONSQUEUE],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[DAEMONSQUEUE]));
	}
	//Fin V1-Ej11-C

	// Return most priority process or NOPROCESS if empty queue
	return selectedProcess; 
}


// Function that assigns the processor to a process
void OperatingSystem_Dispatch(int PID) {

	// The process identified by PID becomes the current executing process
	executingProcessID=PID;//V1-EJ10-B
	int previous = processTable[PID].state;
	// Change the process' state
	processTable[PID].state=EXECUTING;


	// Modify hardware registers with appropriate values for the process identified by PID
	OperatingSystem_RestoreContext(PID);

	//Inicion V1-EJ10-B
	ComputerSystem_DebugMessage(TIMED_MESSAGE,110,SYSPROC,PID,programList[processTable[PID].programListIndex]->executableName,statesNames[previous],statesNames[EXECUTING]);
	//Fin V1-EJ10-B
}


// Modify hardware registers with appropriate values for the process identified by PID
void OperatingSystem_RestoreContext(int PID) {
  
	// New values for the CPU registers are obtained from the PCB
	Processor_PushInSystemStack(processTable[PID].copyOfPCRegister);
	Processor_PushInSystemStack(processTable[PID].copyOfPSWRegister);
	Processor_SetRegisterSP(processTable[PID].copyOfSPRegister);
	//Inicio V1-EJ13
	Processor_SetRegisterA(processTable[PID].copyARegister);
	Processor_SetRegisterB(processTable[PID].copyBRegister);
	Processor_SetAccumulator(processTable[PID].copyAccumulator);
	//Processor_SetCTRL(processTable[PID].copyControl);
	//Fin V1-EJ13
	// Same thing for the MMU registers
	MMU_SetBase(processTable[PID].initialPhysicalAddress);
	MMU_SetLimit(processTable[PID].processSize);
}


// Function invoked when the executing process leaves the CPU 
void OperatingSystem_PreemptRunningProcess() {

	// Save in the process' PCB essential values stored in hardware registers and the system stack
	OperatingSystem_SaveContext(executingProcessID);
	// Change the process' state
	OperatingSystem_MoveToTheREADYState(executingProcessID);
	// The processor is not assigned until the OS selects another process
	executingProcessID=NOPROCESS;
}


// Save in the process' PCB essential values stored in hardware registers and the system stack
void OperatingSystem_SaveContext(int PID) {
	
	// Load PSW saved for interrupt manager
	processTable[PID].copyOfPSWRegister=Processor_PopFromSystemStack();
	
	// Load PC saved for interrupt manager
	processTable[PID].copyOfPCRegister=Processor_PopFromSystemStack();
	
	// Save RegisterSP 
	processTable[PID].copyOfSPRegister=Processor_GetRegisterSP();

	//Inicio V1-EJ13
	processTable[PID].copyARegister=Processor_GetRegisterA();
	processTable[PID].copyBRegister=Processor_GetRegisterB();
	processTable[PID].copyAccumulator=Processor_GetAccumulator();
	//processTable[PID].copyControl=Processor_GetCTRL();
	//Fin V1-EJ13
}


// Exception management routine
void OperatingSystem_HandleException() {
  
	// Show message "Process [executingProcessID] has generated an exception and is terminating\n"
	ComputerSystem_DebugMessage(TIMED_MESSAGE,71,INTERRUPT,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName);
	
	OperatingSystem_TerminateExecutingProcess();
}

// All tasks regarding the removal of the executing process
void OperatingSystem_TerminateExecutingProcess() {

	int previous=processTable[executingProcessID].state;//V1-EJ10-B

	processTable[executingProcessID].state=EXIT;

	//Inicion V1-EJ10-B
	ComputerSystem_DebugMessage(TIMED_MESSAGE,110,SYSPROC,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName,statesNames[previous],statesNames[EXIT]);
	//Fin V1-EJ10-B

	
	if (executingProcessID==sipID) {
		// finishing sipID, change PC to address of OS HALT instruction
		Processor_SetSSP(MAINMEMORYSIZE-1);
		Processor_PushInSystemStack(OS_address_base+1);
		Processor_PushInSystemStack(Processor_GetPSW());
		executingProcessID=NOPROCESS;
		ComputerSystem_DebugMessage(TIMED_MESSAGE,99,SHUTDOWN,"The system will shut down now...\n");
		return; // Don't dispatch any process
	}

	Processor_SetSSP(Processor_GetSSP()+2); // unstack PC and PSW stacked

	if (programList[processTable[executingProcessID].programListIndex]->type==USERPROGRAM) 
		// One more user process that has terminated
		numberOfNotTerminatedUserProcesses--;
	
	if (numberOfNotTerminatedUserProcesses==0) {
		// Simulation must finish, telling sipID to finish
		OperatingSystem_ReadyToShutdown();
	}
	// Select the next process to execute (sipID if no more user processes)
	int selectedProcess=OperatingSystem_ShortTermScheduler();

	// Assign the processor to that process
	OperatingSystem_Dispatch(selectedProcess);
}

// System call management routine
void OperatingSystem_HandleSystemCall() {
  
	int systemCallID;
	int queueId;
	int firstElementPriority;
	int firstElementPID;

	// Register A contains the identifier of the issued system call
	systemCallID=Processor_GetRegisterC();
	
	switch (systemCallID) {
		case SYSCALL_PRINTEXECPID:
			// Show message: "Process [executingProcessID] has the processor assigned\n"
			ComputerSystem_DebugMessage(TIMED_MESSAGE,72,SYSPROC,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName,Processor_GetRegisterA(),Processor_GetRegisterB());
			break;

		case SYSCALL_END:
			// Show message: "Process [executingProcessID] has requested to terminate\n"
			ComputerSystem_DebugMessage(TIMED_MESSAGE,73,SYSPROC,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName);
			OperatingSystem_TerminateExecutingProcess();
			break;
		//Inicio V1-Ej12
		case SYSCALL_YIELD:
			queueId = processTable[executingProcessID].queueID;
			firstElementPriority = processTable[readyToRunQueue[queueId][0].info].priority;
			firstElementPID= readyToRunQueue[queueId][0].info;
			if(processTable[executingProcessID].priority==firstElementPriority && executingProcessID!=firstElementPID){
				OperatingSystem_SaveContext(executingProcessID);
				ComputerSystem_DebugMessage(TIMED_MESSAGE,116,SHORTTERMSCHEDULE,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName,readyToRunQueue[queueId][0].info,programList[processTable[firstElementPID].programListIndex]->executableName);
				OperatingSystem_MoveToTheREADYState(executingProcessID);
				OperatingSystem_Dispatch(OperatingSystem_ExtractFromReadyToRun());
			}else{
				ComputerSystem_DebugMessage(TIMED_MESSAGE,117,SHORTTERMSCHEDULE,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName);
			}
			break;
		//Fin V1-Ej12
	}
}
	
//	Implement interrupt logic calling appropriate interrupt handle
void OperatingSystem_InterruptLogic(int entryPoint){
	switch (entryPoint){
		case SYSCALL_BIT: // SYSCALL_BIT=2
			OperatingSystem_HandleSystemCall();
			break;
		case EXCEPTION_BIT: // EXCEPTION_BIT=6
			OperatingSystem_HandleException();
			break;
	}

}


void  OperatingSystem_PrintReadyToRunQueue(){
	//Inicio V1-Ej9
	//heapItem *queue =readyToRunQueue[0];

	//ComputerSystem_DebugMessage(TIMED_MESSAGE,106,SHORTTERMSCHEDULE);
	//ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,107,SHORTTERMSCHEDULE,queue[0].info,processTable[queue[0].info].priority);
	//for(int i=1;i<numberOfReadyToRunProcesses[0];i++){
	//	ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,108,SHORTTERMSCHEDULE,queue[i].info,processTable[queue[i].info].priority);
	//}
	//ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,109,SHORTTERMSCHEDULE);
	//Fin V1-EJ9

	//Inicio V1-Ej11-B
	/*
	heapItem *userQueue=queue;
	heapItem *daemonQueue=readyToRunQueue[DAEMONSQUEUE];


	if(numberOfReadyToRunProcesses[USERPROCESSQUEUE]>0){
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,114,SHORTTERMSCHEDULE,queueNames[USERPROCESSQUEUE],userQueue[0].info,processTable[userQueue[0].info].priority);
		for(int i=1;i<numberOfReadyToRunProcesses[USERPROCESSQUEUE];i++){
			ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,108,SHORTTERMSCHEDULE,userQueue[i].info,processTable[userQueue[i].info].priority);
		}
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,109,SHORTTERMSCHEDULE);
	}else{
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,115,SHORTTERMSCHEDULE,queueNames[USERPROCESSQUEUE]);
	}

	if(numberOfReadyToRunProcesses[DAEMONSQUEUE]>0){
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,114,SHORTTERMSCHEDULE,queueNames[DAEMONSQUEUE],daemonQueue[0].info,processTable[daemonQueue[0].info].priority);
		for(int i=1;i<numberOfReadyToRunProcesses[DAEMONSQUEUE];i++){
			ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,108,SHORTTERMSCHEDULE,daemonQueue[i].info,processTable[daemonQueue[i].info].priority);
		}
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,109,SHORTTERMSCHEDULE);
	}else{
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,115,SHORTTERMSCHEDULE,queueNames[DAEMONSQUEUE]);
	}*/

	
	for(int queue=0;queue<NUMBEROFQUEUES;queue++){
		if(numberOfReadyToRunProcesses[queue]>0){
			ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,114,SHORTTERMSCHEDULE,queueNames[queue],readyToRunQueue[queue][0].info,processTable[readyToRunQueue[queue][0].info].priority);
		for(int process=1;process<numberOfReadyToRunProcesses[queue];process++){
			ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,108,SHORTTERMSCHEDULE,readyToRunQueue[queue][process].info,processTable[readyToRunQueue[queue][process].info].priority);
		}
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,109,SHORTTERMSCHEDULE);
	}else{
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,115,SHORTTERMSCHEDULE,queueNames[queue]);
	}
	}

	//Fin V1-Ej11-B
}

