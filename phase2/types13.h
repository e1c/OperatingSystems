#ifndef _TYPES13_H
#define _TYPES13_H

#include "uMPStypes.h"

typedef cpu_t U32;
/* Process Control Block (PCB) data structure*/
typedef struct pcb_t {
	/*process queue fields */
	struct pcb_t* p_next;

	/*process tree fields */
	struct pcb_t* p_parent;
	struct pcb_t* p_first_child;
	struct pcb_t* p_sib;

	/* processor state, etc */
	state_t	p_s;

	/* process static priority */
	int	priority;

        /* process dinamic priority */
        int    d_priority;

	/* key of the semaphore on which the process is eventually blocked */
	int	*p_semkey;
        
        /* valore booleano che indica se il processo e' terminato */

        int terminated;

	/* ID CPU che eseguirá il processo*/
	
	U32 cpu_id;

        /* tempo di CPU */
 	cpu_t time;
} pcb_t;

/* Semaphore Descriptor (SEMD) data structure*/
typedef struct semd_t {
	struct semd_t* s_next;

	/* Semaphore key*/
	int *s_key;

	/* Queue of PCBs blocked on the semaphore*/
	pcb_t *s_procQ;
} semd_t;

#endif
