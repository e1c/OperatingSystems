
#include "pcb.e"
#include "asl.e"
#include "libumps.e"
#include "interruptalpha.c"
#include "scheduleralpha.c"


/* Eccezione delle syscall/breakpoint */
void sysBpHandler(){
	
	U32 cpuid = getPRID();

	/* recupero il processo chiamante */
	if(cpuid==0)
        {	state_t *oldProcess = (state_t *)SYSBK_OLDAREA ; }
        else
        {	state_t *oldProcess = &areas[cpuid][SYSBK_OLDAREA_INDEX];	}

	/* prendo i parametri dai registri */
	U32 *n_sys   =  &(oldProcess->reg_a0);
	U32 *arg1    =  &(oldProcess->reg_a1);
	U32 *arg2    =  &(oldProcess->reg_a2);
	U32 *arg3    =  &(oldProcess->reg_a3);

	/* Incremento il PC del processo chiamante della grandezza di una parola (4), per evitare loop */
	oldProcess->pc_epc += WORD_SIZE;
	
	/* recupero lo stato (kernel-user) e il tipo di eccezione */
	U32 *mode  = &(oldProcess->status);
	U32 *cause = &(oldProcess->cause);
	
	/* se mode=kernel */
	if( (*mode & STATUS_KUp) == 0 )
	{
		/*se il processo non ha un handler personalizzato */
		if(currentProcess[cpuid]->custom_handlers[SYSBK_NEWAREA_INDEX] == NULL)
		{		
			/* eseguo la SYSCALL adeguata */
			switch(*n_sys)
			{
				case CREATEPROCESS:
					create_process((state_t*) *arg1, (int) *arg2, (U32) *arg3);
					break;
				case TERMINATEPROCESS:
					terminate_process();
					break;
				case VERHOGEN:
					verhogen((int) *arg1);
					break;
				case PASSEREN:
					passeren((int) *arg1);
					break;
				case SPECTRAPVEC:
					Specify_Exception_State_Vector((int) *arg1, (state_t*) *arg2, (state_t*) *arg3);
					break;
				case GETCPUTIME:
					get_cpu_time();
					break;
				case WAITCLOCK:
					wait_clock();
					break;
				case WAITIO:
					wait_io((int) *arg1, (int) *arg2, (int) *arg3);
					break;
				/* se la SYSCALL non esiste */
				default:
					terminate_process();
					break;
			}
		}
		/* se il processo ha un handler personalizzato */
		else
		{
			/* copio lo stato del processo nel suo handler e poi lo carico */
			cpst(oldProcess, currentProcess[cpuid]->custom_handlers[SYSBK_OLDAREA_INDEX]);
			LDST(currentProcess[cpuid]->custom_handlers[SYSBK_NEWAREA_INDEX]);
		}
	}
	/* se e' in user mode */
	else
	{
		/* passa il controllo al pgmtrap */
		if (cpuid == 0){
			cpst((state_t *)SYSBK_OLDAREA, (state_t *)PGMTRAP_OLDAREA); }
		else {
			cpst(&areas[cpuid][SYSBK_OLDAREA_INDEX], &areas[cpuid][PGMTRAP_OLDAREA_INDEX]); }
		/* settare Cause a EXC_RESERVEDINSTR */
		if (cpuid == 0){
			((state_t *)PGMTRAP_OLDAREA)->cause = EXC_RESERVEDINSTR;
		} else {
			areas[cpuid][PGMTRAP_OLDAREA_INDEX].cause = EXC_RESERVEDINSTR;
		}
		pgmTrapHandler();
	}
}

/* Eccezione del program trap */
void pgmTrapHandler(){
	
	U32 cpuid = getPRID();
	/* processo chiamante */
	if(cpuid==0)
        {	state_t *oldProcess = (state_t *)PGMTRAP_OLDAREA ; }
        else
        {	state_t *oldProcess = &areas[cpuid][PGMTRAP_OLDAREA_INDEX];	}
	
	pcb_t *processoCorrente = currentProcess[cpuid];
	/* se il processo ha un handler personalizzato */
	if(processoCorrente->custom_handlers[PGMTRAP_NEWAREA_INDEX] != NULL)
	{ 
		/* copio lo stato del processo nel suo handler e poi lo carico */
		cpst(oldProcess, processoCorrente->custom_handlers[PGMTRAP_OLDAREA_INDEX]);
		LDST(processoCorrente->custom_handlers[PGMTRAP_NEWAREA_INDEX]); }
	else
	{
		terminate_process();	}	
}

/* Eccezione per la tlb */
void tlbHandler(){
	 
	U32 cpuid = getPRID();

	if(cpuid==0)
        {	state_t *oldProcess = (state_t *)TLB_OLDAREA ; }
        else
        {	state_t *oldProcess = &areas[cpuid][TLB_OLDAREA_INDEX];	}

	pcb_t *processoCorrente = currentProcess[cpuid];
	/* se il processo ha un handler personalizzato */
	if(processoCorrente->custom_handlers[TLB_NEWAREA_INDEX] != NULL)
	{ 
		/* copio lo stato del processo nel suo handler e poi lo carico */
		copyState(oldProcess, processoCorrente->custom_handlers[TLB_OLDAREA_INDEX]);
		LDST(processoCorrente->custom_handlers[TLB_NEWAREA_INDEX]);	}
	else
	{
		terminate_process();	}	
}

/* Eccezione per gli interrupt */
void intHandler(){
	U32 cpuid = getPRID();

	if(cpuid==0)
        {	state_t *oldProcess = (state_t *)INT_OLDAREA ; }
        else
        {	state_t *oldProcess = &areas[cpuid][INT_OLDAREA_INDEX];	}
	/* cerchiamo l'interrupt */
	int intline;
	for (intline= 0; intline < NUM_LINES; intline++){
		/* se intline e' in attesa */
		if (CAUSE_IP_GET(getCAUSE(), intline)){
			break;
		}
	}
	switch(intline){
		case INT_DISK:
			diskint();
		break;
		case INT_TAPE:
			tapeint();
		break;
		case INT_UNUSED:
			unusedint();
		break;
		case INT_PRINTER:
			printerint();
		break;
		case INT_TERMINAL:
			terminalint();
		break;
		default: PANIC();
	}
	scheduler();
}
