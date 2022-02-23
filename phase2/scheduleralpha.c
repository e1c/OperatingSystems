#include "pcb.e"
#include "asl.e"
#include "libumps.e"


void addReadyQ(pcb_t *proc){
	lock(SCHED_LOCK);
	if (proc != NULL)
        proc->d_priority = proc->priority;       
	insertProcQ(&(readyQueue), proc);
	free(SCHED_LOCK);
}

void scheduler() {


	int cpuid = currentProcess->cpu_id /* assegniamo alla variabile il numero del processore nel campo cpu_id del processo;*/
      	currentProcess->time += (GET_TODLOW - procTOD); 
	procTOD = GET_TODLOW;
	lock(SCHED_LOCK); /* bisogna definire SCHED_SEMLOCK come NUM_SEMAPHORES-1 */
        currentProcess[cpuid] = NULL; /* nel caso di WAIT, nessun processo é in esecuzione */

         /* se la readyQ e' vuota */
         if(emptyProcQ(&readyQueue)) /* deadlock detection */
		{
			if(processCount == 0) HALT();
			if((processCount > 0) && (softBlockCount == 0)) PANIC();

			if((processCount > 0) && (softBlockCount > 0))
			{
				/* la CPU va in stato di WAIT: Interrupt attivati e non mascherati */
				setSTATUS((getSTATUS() | STATUS_IEc | STATUS_INT_UNMASKED));
				while(TRUE) ;
			}
			PANIC();
		}
	/* se ci sono processi nella readyQ */	
          else {
		currentProcess[cpuid] = removeProcQ(&(readyQueue)); /* prende un PCB dalla coda ready */
		currentProcess->time = 0;
		aging(); 	/* aumenta la prioritá dinam a tutti i processi rimasti in readyQ */
		procTOD = GET_TODLOW;
                /* se il processo corrente e' terminato */
		if (currentProcess[cpuid]->terminated){
			freePcb(currentProcess[cpuid]);  /* rimette il pcb nella pcbFree */
			processCount--;	
			free(SCHED_LOCK);   /* libera la risorsa */
			LDST(&(scheduler_states[cpuid])); /* ricarica lo scheduler */
		}
               if (!pctInit){
			SET_IT(SCHED_PSEUDO_CLOCK);
			pctInit = TRUE;
		}
                free(SCHED_LOCK);
		setTIMER(SCHED_TIME_SLICE);
		LDST(&(currentProcess[cpuid]->p_s));
	}

        free(SCHED_LOCK);
	WAIT();
	devStatus = 0;  /* cambia lo stato del device terminale */



}
