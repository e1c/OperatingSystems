
void cpst(state_t *src, state_t *dest){
	int i;
        for(i=0;i<29;i++){
			dest->gpr[i]=src->gpr[i];
	}
	dest->entry_hi = src->entry_hi;
	dest->cause = src->cause;
	dest->status = src->status;
	dest->pc_epc = src->pc_epc;
	dest->hi = src->hi;
	dest->lo = src->lo;
}

/* se il semaforo key e' uguale a 1 (accessibile) allora accedi alla risorsa e termina il while
   altrimenti il ciclo continua */
void lock(int key)
{
	while(!CAS(&locks[key],1,0));
}


void free(int key)
{
	CAS(&locks[key],0,1);
}

/* incrementa di 1 la prioritá dinamica */
void incrementa(pcb_t* p)
{
if(p->d_priority==19) return;
else p->d_priority++;
}

/* chiama la funz incrementa su tutti i processi nella readyQ */
void aging()
{
forallProcQ(readyQ, &incrementa);
}

void passerenIO(int *semaddr)
{
	(*semaddr)--;

	if((*semaddr) < 0)
	{
		/* Inserisce il processo corrente in coda al semaforo specificato */
		if(insertBlocked((S32 *) semaddr, currentProcess)) PANIC(); /* PANIC se sono finiti i descrittori dei semafori */
		currentProcess->p_isOnDev = IS_ON_DEV;
		currentProcess = NULL;
		softBlockCount++;
		scheduler();
	}
}

/* trova il primo device in attesa,e restituisce il suo numero */
int dev_num(U32 bitmap){
	int i=0;
	for(i=0; i<WORD_SIZE; i++){
	/* ad ogni ciclo shifta 1 a sinistra e confronta in AND il bitmap */
		if ((1<<i) & bitmap) return i; }
}
