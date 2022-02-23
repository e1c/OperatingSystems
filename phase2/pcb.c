#include "const13.h"
#include "types13.h"
#include "pcb.e"

pcb_t pcb_table[MAXPROC];
pcb_t* pcbfree_h;

/*  checks if a list is empty  */
HIDDEN int pcb_lista_vuota(pcb_t* head){	
	if (head==NULL) return 1; 	/*  true */
	else return 0; 			/*  false */
}

/*  sets all values of gpr-array to 0  */
HIDDEN void init_gpr(pcb_t* p, int i){
		if (i == 29) return;
		else {
			p->p_s.gpr[i]=0;
		init_gpr(p, i+1);
	}

}

/*  initialise a pcb with the fields set to zero/null */
HIDDEN void init_pcb(pcb_t *p){
		p->p_next=NULL;
		p->p_parent=NULL;
		p->p_first_child=NULL;
		p->p_sib=NULL;
		p->p_semkey=NULL;
                p->priority=0;
		p->d_priority = p->priority;
		p->terminated = FALSE;
                p->time = 0;
}

/* initialise the pcbFree so that it contains all of the ellements of the  pcb-table. we call it only once. */

/* auxiliary function for initPcbs */
HIDDEN void PTabletoList(int i)
{
if (i==MAXPROC-1)
	{
     	pcb_table[i].p_next=NULL;return;  /* checks if this is the last field of the pcb_table */
	}
else
	{
     	pcb_table[i].p_next = &(pcb_table[i+1]); /* it concatenates the elements of the table with each other assigning the address at the p_next */
     	i=i+1;
     	PTabletoList(i);
	}    
}
/* Initialize the pcbFree list to contain all the elements of the static array of MAXPROC ProcBlk’s. This method will be called
only once during data structure initialization. */

void initPcbs(){
    	pcbfree_h = &(pcb_table[0]); /*pcbfree_h points at the first element of the table*/
        PTabletoList(0);
}

/* ------------------------------------------------------------------------------------------ */
/* Insert the element pointed to by p into the pcbFree list. */

void freePcb(pcb_t *p){
       
    	if (p == NULL)		return;
    	else{ 
   		pcb_t* temp = pcbfree_h;	/*  assign to temp the actual first element of pcbFree */
		pcbfree_h = p;
		pcbfree_h->p_next = temp;
		}
}

/* Return NULL if the pcbFree list is empty. Otherwise, remove an element from the pcbFree list, provide initial values for ALL
of the ProcBlk’s fields (i.e. NULL and/or 0) and then return a pointer to the removed element.  */

pcb_t *allocPcb(){
	if ( pcb_lista_vuota(pcbfree_h) ) return NULL;
	else{	
		pcb_t* temp = pcbfree_h;		/*  assign to temp the first pcb of the pcbFree list */
		pcbfree_h = pcbfree_h->p_next;		/*  make pcb_free to point at the second element of the list*/
		init_pcb(temp);
		return temp;
	}
}

/* ----------------------------------------------------------------------------------------- */
/* Insert the ProcBlk pointed to by p into the process queue whose tail-pointer is pointed to by tp. Note the double indirection through
tp to allow for the possible updating of the tail pointer as well. */

void insertProcQ(pcb_t **head, pcb_t *p){
    
    /* if the list is empty i insert the element as the head */
    	if ( pcb_lista_vuota(*head) ) {*head=p;	return;}

    /* if the element has a major priority comparing to head,insert it at the actual position */
        else if ( p->d_priority > (*head)->d_priority) {	
			pcb_t* temp = *head;
			*head = p;
			p->p_next = temp;
		}
    /*  if the priority is less than or equal to,it goes on with recursion */
        else { insertProcQ( &((*head)->p_next),p );}
    }  

/* ---------------------------------------------------------------------------------------- */
/* Return a pointer to the first ProcBlk from the process queue whose tail is pointed to by tp. Do not remove this ProcBlkfrom the process
queue. Return NULL if the process queue is empty. */

pcb_t *headProcQ(pcb_t* head){
	return head;
	
}

/* ----------------------------------------------------------------------------------------- */
/* Remove the first (i.e. head) element from the process queue whose tail-pointer is pointed to by tp. Return NULL if the process queue
was initially empty; otherwise return the pointer to the removed element. Update the process queue’s tail pointer if necessary. */

pcb_t* removeProcQ(pcb_t** head){
	if ( pcb_lista_vuota(*head) ) return NULL;
	else{
		pcb_t* temp = *head;		/*  assign to temp the first element of the list pointed by head */
		*head = (*head)->p_next;	/*  head points at the second element now  */
		temp->p_next=NULL;
		return temp;
		}
}

/* ------------------------------------------------------------------------------------------- */

/* Remove the ProcBlk pointed to by p from the process queue whose tail-pointer is pointed to by tp. Update the process queue’s tail
pointer if necessary. If the desired entry is not in the indicated queue (an error condition), return NULL; otherwise, return p. Note that p
can point to any element of the process queue. */

pcb_t* outProcQ( pcb_t** head, pcb_t *p){
	
	if ( pcb_lista_vuota(*head) )                 /* if the list pointe by head is empty,return null */
	    return NULL;
	
            else {
				if (*head==p)                      /* if the first element is equal to p we assign to the first the address of the second */
				{ *head=(*head)->p_next; return (p);}
            
                else {return outProcQ(&((*head)->p_next),p);} /* else we recall the function with the p_next */
                            
                            
				}
}
			
/* ---------------------------------------------------------------------- */

void forallProcQ(struct pcb_t *head, void fun(struct pcb_t *pcb, void *), void *arg){
  if (head!=NULL) {
                                        (*fun)(head,arg); /*apply the function to head when not null*/
                                        forallProcQ(head->p_next,(*fun),arg); /*recall the function for p_next*/
                                        }
  else return;

}



/* function that inserts the pcb as the last child of p1 */

HIDDEN void insertLastChild(pcb_t* p1, pcb_t* p){
	if (p1->p_sib == NULL) {p1->p_sib = p; return;} /*searches a pcb that doesnt have a brother and inserts it as its brother*/
	else insertLastChild(p1->p_sib, p);
		}
/* Make the ProcBlk pointed to by p a child of the ProcBlk pointed to by prnt. */

void insertChild(pcb_t *parent, pcb_t *p){
	if (parent->p_first_child == NULL) parent->p_first_child=p;
	else insertLastChild(parent->p_first_child, p);
	
	p->p_parent=parent; /*assign parent as the parent of p,and also delete his brother since he is the last child*/
	p->p_sib=NULL;
}

/* ------------------------------------------------------------------------------------------ */


/* Make the first child of the ProcBlk pointed to by p no longer a child of p. Return NULL if initially there were no children of p.
Otherwise, return a pointer to this removed first child ProcBlk. */

pcb_t* removeChild(pcb_t *p){
		if (p->p_first_child == NULL) return NULL;
		else {
			pcb_t* temp = p->p_first_child;	/*  save in temp the first child */
			pcb_t* new_first= temp->p_sib;	/*  ,assign to new_first the new  first child of p,the brother of the previous first child */
			p->p_first_child = new_first; 	
			temp->p_parent=NULL;		/*  the removed child doesn't have a father anymore */
			temp->p_sib=NULL;
			return temp;
			
		}
}

/* ------------------------------------------------------------------------------------------- */

/* support function: ,takes in input a child "son" (at the first call the son will be the second_child), */
HIDDEN pcb_t* scan_sibling(pcb_t* son,pcb_t* prec_son,pcb_t* target_son){
	if (son == target_son) {	
		prec_son->p_sib=target_son->p_sib;	/*  the previous child points now at the second one,that becomes now the first child */
		target_son->p_sib=NULL;			/*  ,now that its removed target doesnt have a sibling and neither a father */
		target_son->p_parent=NULL;	
		return target_son;			
	}
	else return scan_sibling(son->p_sib, son, target_son);
		
}
/* Make the ProcBlk pointed to by p no longer the child of its parent. If the ProcBlk pointed to by p has no parent, return NULL; otherwise,return p. Note that the element pointed to by p need not be the first
child of its parent. */

pcb_t* outChild(pcb_t* p){
	if (p==NULL) return NULL;
	else{
	if ( (p->p_parent)==NULL ) return NULL;
	else {	pcb_t* father = p->p_parent;				/*  in father save the father */
		pcb_t* first = father->p_first_child; 			/*  it saves the pointer to the first_child at the variable first */
		if( first == p) return removeChild(p->p_parent);	/*  ,if the child to be rimoved is the first, uses the function removeChild() by giving father in input */
		else {	pcb_t* second = first->p_sib;			
			return scan_sibling(second,first, p);		/*  uses scan_sibling, by giving in input the sibling and the target */
		}
	}  
    }
}


