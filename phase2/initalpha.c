
#include "pcb.e"
#include "asl.e"
#include "libumps.e"
#include "scheduleralpha.c"

HIDDEN void popul_area(memaddr area, memaddr handler)
{
	state_t *new_area;
	
	/* la nuova area punta alla vecchia */
	new_area = (state_t *) area;
	
/* epc é il pc per le eccezioni, e t9 punta sempre all'inizio della procedura (all'inizio, epc e t9 puntano allo stesso indirizzo) */
        /* imposta reg_sp a RAMTOP */
	/* inizializza il registro status a: interrupt mascherati, memoria virtuale OFF, kernel mode ON */
	new_area->pc_epc = new_area->reg_t9 = handler;
	new_area->reg_sp = RAMTOP;
	new_area->status = (newArea->status | STATUS_KUc) & ~STATUS_INT_UNMASKED & ~STATUS_VMp;
}

/* variabili del kernel */
struct {
	int disk[DEV_PER_INT];
	int tape[DEV_PER_INT];
	int network[DEV_PER_INT];
	int printer[DEV_PER_INT];
	int terminalR[DEV_PER_INT];
	int terminalT[DEV_PER_INT];
} sem;

int processCount; /* contatore dei processi */
int softBlockCount; /* contatore dei processi bloccati */
pcb_t *readyQ; /* coda dei processi ready */
pcb_t *currentProcess[MAX_CPU]; /* puntatore al processo in esecuzione */
state_t areas[MAX_CPU][NUM_AREAS]; /* Aree reali per CPU > 0 */
int locks[NUM_SEMAPHORES]; /* Variabili di condizione per CAS */
state_t scheduler_states[MAX_CPU]; /* state_t dello scheduler */
int pctInit; /* Lo Pseudo Clock Timer è stato inizializzato? */
U32 devStatus; /* Status in output dei vari device (solo uno perché solo il terminale è usato) */
cpu_t procTOD; /* Tempo d'avvio del processo corrente sul processore */

/* Dichiarazione esterna della funzione test() */
extern void test(void);

int main(void)
{
        pcb_t *init;
	int i,cpuid;
	
	/* popolazione delle new area */
        popul_area(INT_NEWAREA, (memaddr) intHandler);
        popul_area(TLB_NEWAREA, (memaddr) tlbHandler);
	popul_area(SYSBK_NEWAREA, (memaddr) sysBpHandler);
	popul_area(PGMTRAP_NEWAREA, (memaddr) pgmTrapHandler);
	
	/* funzioni phase 1*/
        initPcbs();	/* crea lista pcbFree, contenete tutti i pcb_t */
        initASL();	/* crea la lista ASL, contenete tutti i semafori */

	readyQ = NULL; /* processi ready null */

       /* inizializziamo gli stati dello scheduler */
	for(cpuid=0;cpuid<NUM_CPU;cpuid++){
                 STST(&(scheduler_states[cpuid]));
                 scheduler_states[cpuid].reg_sp = SFRAMES_START-(cpuid*FRAME_SIZE);
		/* assegna al PC di ogni CPU lo scheduler */
                 scheduler_states[cpuid].pc_epc = scheduler_states[cpuid].reg_t9 = (memaddr)scheduler;
                 /* Il TIMER e' disabilitato durante l'esecuzione dello scheduler */
                 scheduler_states[cpuid].status = SCHEDULER_STATUS;
            }

	processCount = 0;
	softBlockCoutn = 0;
	
	for (i=0; i<MAX_CPU;i++) { currentProcess[i]=NULL;} /* processo corrente a null per ogni cpu */
       
        init = allocPcb(); /* estrae il primo pcb dalla pcbFree */

        if(init==NULL) PANIC();	/* se non ci sono processi... */

        init->p_state.status = (init->p_state.status | STATUS_IEp | STATUS_INT_UNMASKED | STATUS_KUc) & ~STATUS_VMp;
        init->p_state.reg_sp = RAMTOP - FRAME_SIZE;
        init->p_state.pc_epc = init->p_state.reg_t9 = (memaddr)test;

        insertProcQ(&readyQ, init); /* inserisce il pcb estratto prima, nella coda readyQ */

	processCount++; /* aumenta il contatore dei processi */

	for(cpuid=1;cpuid<NUM_CPU;cpuid++){
		/* inizializza cpuid allo stato scheduler_states, e   */
             	INITCPU(cpuid, &scheduler_states[cpuid], &areas[cpuid]);
           }

	LDST(&(scheduler_states[0]));
        return 0;

}
