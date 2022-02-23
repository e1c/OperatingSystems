#include "const13.h"
#include "types13.h"
#include "pcb.e"

pcb_t pcb_table[MAXPROC];
pcb_t* pcbfree_h;

/*  controlla se una lista di pcb é vuota  */
HIDDEN int pcb_lista_vuota(pcb_t* head){	
	if (head==NULL) return 1; 	/*  vero */
	else return 0; 			/*  falso */
}

/*  sets all values of gpr-array to 0  */
HIDDEN void init_gpr(pcb_t* p, int i){
		if (i == 29) return;
		else {
			p->p_s.gpr[i]=0;
		init_gpr(p, i+1);
	}

}

/*  inzializza un pcb_t a zero/NULL */
HIDDEN void init_pcb(pcb_t *p){
		p->p_next=NULL;
		p->p_parent=NULL;
		p->p_first_child=NULL;
		p->p_sib=NULL;
		
		p->p_s.entry_hi=0;
		p->p_s.cause=0;
		p->p_s.status=0;
		p->p_s.pc_epc=0;  /* pc in the new area, epc in the old area */
		init_gpr(p,0);
		p->p_s.hi=0;
		p->p_s.lo=0;

		p->priority=0;
		p->p_semkey=NULL;
}

/* Inizializza la pcbFree in modo da contenere tutti gli elementi della pcb-table. Viene chiamato una sola volta. */

/* auxiliary function for initPcbs */
HIDDEN void PTabletoList(int i)
{
if (i==MAXPROC-1)
	{
     	pcb_table[i].p_next=NULL;return;
	}
else
	{
     	pcb_table[i].p_next = &(pcb_table[i+1]);
     	i=i+1;
     	PTabletoList(i);
	}    
}

void initPcbs(){
    	pcbfree_h = &(pcb_table[0]);
        PTabletoList(0);
}

/* ------------------------------------------------------------------------------------------ */
/* Inserisce il Pcb puntato da p nella lista dei Pcb liberi (pcbFree) */
void freePcb(pcb_t *p){
       
    	if (p == NULL)		return;
    	else{ 
   		pcb_t* temp = pcbfree_h;	/*  assegno a temp l'attuale primo elem della pcbFree */
		pcbfree_h = p;
		pcbfree_h->p_next = temp;
		}
}

/*  rimuove e restituisce il primo elem dalla lista pcbFree e ne inizializza i campi a zero/NULL, oppure restuituisce NULL se vuota */
pcb_t *allocPcb(){
	if ( pcb_lista_vuota(pcbfree_h) ) return NULL;
	else{	
		pcb_t* temp = pcbfree_h;		/*  assegnamo a temp il primo pcb nella lista pcbFree */
		pcbfree_h = pcbfree_h->p_next;		/*  facciamo puntare pcbfree_h al secondo elemento, per togliere il primo dalla lista */
		init_pcb(temp);
		return temp;
	}
}

/* ----------------------------------------------------------------------------------------- */

void insertProcQ(pcb_t **head, pcb_t *p){
    
    /* Se la lista è vuota inserisco l'elemento in testa alla lista */
    	if ( pcb_lista_vuota(*head) ) {*head=p;	return;}

    /* Se l'elemento p ha una priorità maggiore dell'elemento di testa, lo inserisco nell'attuale posizione */
        else if ( p->priority > (*head)->priority) {	
			pcb_t* temp = *head;
			*head = p;
			p->p_next = temp;
		}
    /*  Se la prioritá non é maggiore (quindi uguale o minore), si va avanti con ricorsione */
        else { insertProcQ( &((*head)->p_next),p );}
    }  

/* ---------------------------------------------------------------------------------------- */

/*  prende il primo elem della lista puntata da head, oppure restituisce NULL se vuota */
pcb_t *headProcQ(pcb_t* head){
	pcb_t* proc;
	proc=head;
	if (pcb_lista_vuota(proc)) return NULL;
	else return proc;
	
}

/* ----------------------------------------------------------------------------------------- */

pcb_t* removeProcQ(pcb_t** head){
	if ( pcb_lista_vuota(*head) ) return NULL;
	else{
		pcb_t* temp = *head;		/*  assegnamo a temp il primo pcb nella lista puntata da head */
		*head = (*head)->p_next;	/*  facciamo puntare head al secondo elemento, per togliere il primo dalla lista */
		temp->p_next=NULL;
		return temp;
		}
}

/* ------------------------------------------------------------------------------------------- */

/*DESCRIZIONE: Rimuove il PCB puntato da p dalla coda dei processi puntata da head. Se p non e’ presente nella coda, restituisce NULL. 
(NOTA: p puo’ trovarsi in una posizione arbitraria della coda).	*/
pcb_t* outProcQ( pcb_t** head, pcb_t *p){
	
	if ( pcb_lista_vuota(*head) )                 /* se la lista puntata da head è vuota ,ritorna null */
	    return NULL;
	
            else {
				if (*head==p)                      /* se il primo elemento è uguale a p ,assegniamo al primo ,l'indirizzo del secondo */
				{ *head=(*head)->p_next; return (p);}
            
                else {return outProcQ(&((*head)->p_next),p);} /* else richiamiamo la funzione e gli diamo in pasto il campo p_next di head */
                            
                            
				}
}
			
/* ---------------------------------------------------------------------- */

void forallProcQ(struct pcb_t *head, void fun(struct pcb_t *pcb, void *), void *arg){
  if (head!=NULL) {
                                        (*fun)(head,arg);
                                        forallProcQ(head->p_next,(*fun),arg);
                                        }
  else return;

}



/* inserisce il pcb puntato da p come figlio del pcb puntato da parent */

HIDDEN void insertLastChild(pcb_t* p1, pcb_t* p){
	if (p1->p_sib == NULL) {p1->p_sib = p; return;}
	else insertLastChild(p1->p_sib, p);
		}

void insertChild(pcb_t *parent, pcb_t *p){
	if (parent->p_first_child == NULL) parent->p_first_child=p;
	else insertLastChild(parent->p_first_child, p);
	
	p->p_parent=parent;
}

/* ------------------------------------------------------------------------------------------ */


/*  Rimuove il primo figlio del PCB puntato da p. Se p non ha figli, restituisce NULL, altrimenti restituisce il figlio rimosso */
pcb_t* removeChild(pcb_t *p){
		if (p->p_first_child == NULL) return NULL;
		else {
			pcb_t* temp = p->p_first_child;	/*  salvo in temp la primo figlio */
			pcb_t* new_first= temp->p_sib;	/*  assegno a new_first il nuovo primo di figlio di p, cioé il fratello del precedente primo figlio */
			p->p_first_child = new_first; 	/*  ora il padre ha un primo figlio diverso, e il precedente primo figlio é rimosso */
			temp->p_parent=NULL;		/*  il figlio rimosso non ha piú il padre */
			temp->p_sib=NULL;
			return temp;
			
		}
}

/* ------------------------------------------------------------------------------------------- */

/* funzione di supporto: prende in input un figlio son (alla prima chiamata son sará il secondo figlio),il precedente fratello, e il figlio cercato
il figlio precedente mi serve per farlo puntare al figlio successivo di quello che toglieró */
HIDDEN pcb_t* scan_sibling(pcb_t* son,pcb_t* prec_son,pcb_t* target_son){
	if (son == target_son) {	
		prec_son->p_sib=target_son->p_sib;	/*  il precedente filgio punta al successivo, saltando (quindi rimuovendo) il figlio target */
		target_son->p_sib=NULL;			/*  essendo rimosso, target non ha piú un fratello succ, e neanche un padre */
		target_son->p_parent=NULL;	
		return target_son;			
	}
	else return scan_sibling(son->p_sib, son, target_son);
		
}

/* Rimuove il PCB puntato da p dalla lista dei figli del padre. Se il PCB puntato da p non ha un padre, restituisce NULL. 
Altrimenti restituisce l’elemento rimosso (cioe’ p). A differenza della removeChild, p puo’trovarsi in una posizione arbitraria (ossia non e’
necessariamente il primo figlio del padre).	*/
pcb_t* outChild(pcb_t* p){
	if (p==NULL) return NULL;
	else{
	if ( (p->p_parent)==NULL ) return NULL;
	else {	pcb_t* father = p->p_parent;				/*  in father salvo il padre */
		pcb_t* first = father->p_first_child; 			/*  nella var first salvo il puntatore al primo figlio */
		if( first == p) return removeChild(p->p_parent);	/*  se il figlio da rimuovere é il primo, uso la funz removeChild() con padre in input */
		else {	pcb_t* second = first->p_sib;			/*  il secondo figlio */
			return scan_sibling(second,first, p);		/*  uso scan_sibling, dando in input il secondo figlio e il target p */
		}
	}  
    }
}

/* -------------------------------------------------------------------------------------------- */
