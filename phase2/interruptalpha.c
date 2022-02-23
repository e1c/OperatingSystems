
#include "pcb.e"
#include "asl.e"
#include "libumps.e"

void diskint() {
        /* Calcolo il numero del device che ha generato l'interrupt */
	int dnum = dev_num(INTR_CURRENT_BITMAP(INT_DISK));

	/* Calcoliamo l'inizio del registro del controller per scrivere/leggere sui suoi registri */
	int devaddress = DEV_ADDRESS(INT_DISK, dnum); //cambiare nomi variabili!

	/* indice del semaforo da usare */
	int dsem = &(sem.disk[dnum]);

	dtpreg_t *diskreg = (dtpreg_t *)devaddress;

	semd_t *diskSem = getSemd(dsem);
	/* puntatore al primo processo bloccato sulla coda */
	pcb_t *waitingProc = headBlocked(dsem);

	if (waitingProc != NULL){
		/* Maggior priorità alla trasmissione */
		/* scrivo la risposta nel processo in coda */
		waitingProc->p_s.reg_v0 = diskreg->status;
	} else {
		/* scrivo lo status nel vettore, il processo non
		 * ha ancora fatto la P */
		devStatus = diskreg->status;
	}
	verhogen(dsem);
	/* acknowledgement al device */
	diskreg->command = DEV_C_ACK;
}

void tapeint() {
        /* Calcolo il numero del device che ha generato l'interrupt */
	int dnum = dev_num(INTR_CURRENT_BITMAP(INT_TAPE));

	/* Calcoliamo l'inizio del registro del controller per scrivere/leggere sui suoi registri */
	int devaddress = DEV_ADDRESS(INT_TAPE, dnum); //cambiare nomi variabili!

	/* indice del semaforo da usare */
	int tsem = &(sem.tape[dnum]);

	dtpreg_t *tapereg = (dtpreg_t *)devaddress;

	semd_t *tapeSem = getSemd(tsem);
	/* puntatore al primo processo bloccato sulla coda */
	pcb_t *waitingProc = headBlocked(tsem);

	if (waitingProc != NULL){
		/* Maggior priorità alla trasmissione */
		/* scrivo la risposta nel processo in coda */
		waitingProc->p_s.reg_v0 = tapereg->status;
	} else {
		/* scrivo lo status nel vettore, il processo non
		 * ha ancora fatto la P */
		devStatus = tapereg->status;
	}
	verhogen(tsem);
	/* acknowledgement al device */
	tapereg->command = DEV_C_ACK;
}

void printerint() {
        /* Calcolo il numero del device che ha generato l'interrupt */
	int dnum = dev_num(INTR_CURRENT_BITMAP(INT_PRINTER));

	/* Calcoliamo l'inizio del registro del controller per scrivere/leggere sui suoi registri */
	int devaddress = DEV_ADDRESS(INT_PRINTER, dnum); //cambiare nomi variabili!

	/* indice del semaforo da usare */
	int psem = &(sem.printer[dnum]);

	dtpreg_t *printerreg = (dtpreg_t *)devaddress;

	semd_t *printerSem = getSemd(psem);
	/* puntatore al primo processo bloccato sulla coda */
	pcb_t *waitingProc = headBlocked(psem);

	if (waitingProc != NULL){
		/* Maggior priorità alla trasmissione */
		/* scrivo la risposta nel processo in coda */
		waitingProc->p_s.reg_v0 = printerreg->status;
	} else {
		/* scrivo lo status nel vettore, il processo non
		 * ha ancora fatto la P */
		devStatus = printerreg->status;
	}
	verhogen(psem);
	/* acknowledgement al device */
	printerreg->command = DEV_C_ACK;
}

void unusedint() {
        /* Calcolo il numero del device che ha generato l'interrupt */
	int dnum = dev_num(INTR_CURRENT_BITMAP(INT_UNUSED));

	/* Calcoliamo l'inizio del registro del controller per scrivere/leggere sui suoi registri */
	int devaddress = DEV_ADDRESS(INT_UNUSED, dnum); //cambiare nomi variabili!

	/* indice del semaforo da usare */
	int usem = &(sem.network[dnum]);

	dtpreg_t *unusedreg = (dtpreg_t *)devaddress;

	semd_t *unusedSem = getSemd(usem);
	/* puntatore al primo processo bloccato sulla coda */
	pcb_t *waitingProc = headBlocked(usem);

	if (waitingProc != NULL){
		/* Maggior priorità alla trasmissione */
		/* scrivo la risposta nel processo in coda */
		waitingProc->p_s.reg_v0 = unusedreg->status;
	} else {
		/* scrivo lo status nel vettore, il processo non
		 * ha ancora fatto la P */
		devStatus = unusedreg->status;
	}
	verhogen(usem);
	/* acknowledgement al device */
	unusedreg->command = DEV_C_ACK;
}

void terminalint() {

	/* Calcolo il numero del device che ha generato l'interrupt */
	int dnum = dev_num(INTR_CURRENT_BITMAP(INT_TERMINAL));

	/* Calcoliamo l'inizio del registro del controller per scrivere/leggere sui suoi registri */
	int devaddress = DEV_ADDRESS(INT_TERMINAL, dnum); //cambiare nomi variabili!

	/* indice del semaforo da usare */
	int tsem = &(sem.terminalR[dnum]);
	
	termreg_t *termreg = (termreg_t *)devaddress;
	/* indice del vettore delle risposte associato al device */
//	int termStatusNo = GET_TERM_STATUS(INT_TERMINAL, dnum, FALSE); SIAMO *********ARRIVATI QUI!

	semd_t *termSem = getSemd(tsem);
	/* puntatore al primo processo bloccato sulla coda */
	pcb_t *waitingProc = headBlocked(tsem);
	/* se non ci sono processi in coda */
	if (waitingProc != NULL){
		/* Maggior priorità alla trasmissione */
		/* scrivo la risposta nel processo in coda */
		waitingProc->p_s.reg_v0 = termreg->transm_status;
	} else {
		/* scrivo lo status nel vettore, il processo non
		 * ha ancora fatto la P */
		devStatus = termreg->transm_status;
	}
	verhogen(tsem);
	/* acknowledgement al device */
	termreg->transm_command = DEV_C_ACK;

}

