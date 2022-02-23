#include "pcb.e"
#include "asl.e"
#include "libumps.e"
#include "utilfunctions.c"

/*
Quando invocata,la SYS1 determina la 
creazione di un processo figlio del processo chiamante*/	
  
int create_process(state_t *statep, int priority, U32 cpuid){
	
	int cpuid_new = cpuid%GET_NCPU;/* calcolo a quale cpu assegnerá il processo */

        /* se la priorita' non e' valida */
        if(priority>19 || priority<0)
        { /*program trap*/ }
	/* se l'id del processore e'uguale a zero gli si assegna la old_area corrente nella sysbk
	else andiamo a cercare nella matrice */
        if(cpuid_new==0)
        {
        state_t *oldProcess = (state_t *)SYSBK_OLDAREA ; 
        }
        else
        {
        state_t *oldProcess = &areas[cpuid_new][SYSBK_OLDAREA_INDEX];
        }
	/* ottengo il processo corrente */
	pcb_t *processoCorrente = currentProcess[cpuid_new];
	/* alloco un nuovo processo */
	pcb_t *nuovoProcesso = allocPcb();
	/* se non è possibile allocare un nuovo processo */
	if(nuovoProcesso == NULL)
	{
		return -1;	
		//LDST(&(oldProcess));
	}
	cpst(statep, &(nuovoProcesso->p_s));
	nuovoProcesso->priority = priority;
	nuovoProcesso->cpu_id = cpuid_new;
	insertChild(processoCorrente, nuovoProcesso);
	addReady(nuovoProcesso);
	processCount++;
	return 0;
}

/*---------------SYS 2--------------------*/ 
/*  Terminate Process*/

void terminate_process()
{
	U32 cpuid = getPRID(); /* Id processore*/
	
	pcb_t *processoCorrente = currentProcess[cpuid]; /* Id processo */

	/* elimino il processo e tutti i discendenti, e imposta terminated=TRUE a tutti */
	outChildBlocked(processoCorrente); 

	freePcb(processoCorrente);
	processCount--;
	scheduler();
}

/*---------------SYS 3--------------------*/ 
/*  Verhogen */

void verhogen(int semKey){
	
	lock(semKey);
	U32 cpuid = getPRID();
	semd_t *sem = getSemd(semKey);

	if(cpuid==0)
        {	state_t *oldProcess = (state_t *)SYSBK_OLDAREA ; }
        else
        {	state_t *oldProcess = &areas[cpuid][SYSBK_OLDAREA_INDEX];	}

	*(sem->s_key) += 1;
	if (*(sem->s_key) <= 0){
		pcb_t *toWake = removeBlocked(semKey);
		if (toWake != NULL){
			addReady(toWake); // sveglio il prossimo
		}
	}
	free(semKey);
	LDST(oldProcess);
}

/*---------------SYS 4--------------------*/ 
/*  Passeren */
void passeren(int semKey){
	
	lock(semKey);
	int cpuid = getPRID();

	if(cpuid==0)
        {	state_t *oldProcess = (state_t *)SYSBK_OLDAREA ; }
        else
        {	state_t *oldProcess = &areas[cpuid][SYSBK_OLDAREA_INDEX];	}

	semd_t *semaphore = getSemd(semKey);
	semaphore->s_value -= 1;
	if (semaphore->s_value < 0){
		cpst(oldProcess, &(currentProcess[cpuid]->p_s));
		insertBlocked(semKey, currentProcess[cpuid]);
		free(semKey);
		LDST(&(scheduler_states[cpuid]));
	} else {
		free(semKey);
		LDST(oldProcess);
	}
	
}


/*---------------SYS 5--------------------*/ 
/*  Specify Exception State Vector */
Specify_Exception_State_Vector(int type, state t *oldp,state t *newp){	

	U32 cpuid = getPRID();
	switch (type){
	case 0: 	/* copio i custom handlers nel pcb_t del processo chiamante*/
		currentProcess[cpuid]->custom_handlers[TLB_NEWAREA_INDEX] = newp;
		currentProcess[cpuid]->custom_handlers[TLB_OLDAREA_INDEX] = oldp;
		break;
	case 1: 	
		currentProcess[cpuid]->custom_handlers[PGMTRAP_NEWAREA_INDEX] = newp;
		currentProcess[cpuid]->custom_handlers[PGMTRAP_OLDAREA_INDEX] = oldp;
		break;
	case 2: 	
		currentProcess[cpuid]->custom_handlers[SYSBK_NEWAREA_INDEX] = newp;
		currentProcess[cpuid]->custom_handlers[SYSBK_OLDAREA_INDEX] = oldp;
		break;
       default: 
      		terminate_process();
	}/* fine switch*/
}

/*---------------SYS 6--------------------*/ 
/* Get CPU Time */

int get_cpu_time()
{
	/* Aggiorna il tempo di vita del processo sulla CPU */
	currentProcess->p_cpu_time += (GET_TODLOW - processTOD);
	processTOD = GET_TODLOW;
	
	return currentProcess->time;
}

/*---------------SYS 7--------------------*/ 
/* Wait Clock */
void wait_clock()
{
	/* Chiamiamo una P sul semaforo dedicato al PCT */
	passeren(CLOCK_SEM);
}

/*---------------SYS 8--------------------*/ 
/* Wait IO */
/* input: linea di interrupt, numero di device, operazione di terminal read/write	
 */

	unsigned int wait_io(int intline, int dnum, int waitForTermRead)
{
	switch(intline)
	{
		case INT_DISK:
			passeren(&sem.disk[dnum]);
		break;
		case INT_TAPE:
			passeren(&sem.tape[dnum]);
		break;
		case INT_UNUSED:
			passeren(&sem.network[dnum]);
		break;
		case INT_PRINTER:
			passeren(&sem.printer[dnum]);
		break;
		case INT_TERMINAL:
			if(waitForTermRead)
				passeren(&sem.terminalR[dnum]);
			else
				passeren(&sem.terminalT[dnum]);
		break;
		default: PANIC();
	}
	
	return devStatus;
}




