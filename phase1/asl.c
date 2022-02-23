/* 
This file is created by Giorgi Riccardo, for the management of semaphores.

This is a part of SOS O.S., an evolution of Kaya O.S., itself
evolution of a long list of O.S. proposed for educational
purposes (HOCA, TINA, ICARUS, etc)
*/

#include "asl.e"
#include "pcb.e"

/* array for all the semaphores */
semd_t semd_table[MAXPROC];
/* head of the Free List */
semd_t *semdFree_h;
/* head of the ASL (Active Semaphore List) */
semd_t *semd_h;

/****************AUXILIARY FUNCIONTS****************/

/* auxiliary function for find the right semd_t from queue semd_f having ID=key: returns the semd_t found, NULL if not found */
HIDDEN semd_t* find(int *key, semd_t *semd_f)
{
        if(semd_f==NULL)
        {
            return NULL;
        }
            else
           {
        if(semd_f->s_key==key)
        {
            return semd_f;
        }
        else
        {
            return find(key, semd_f->s_next);
        }
          }
            
}

/* moves a semd_t from Free List to ASL: returns 1 if success, 0 if free list is empty */
HIDDEN int FreetoASL(int* key)
{
        if (semdFree_h==NULL)
        {
        return 0;
        }
        else
        {
        semd_t* temp=semd_h;
        semd_h=semdFree_h;
        semdFree_h=semdFree_h->s_next;
        semd_h->s_next=temp;
        semd_h->s_key=key;
        return 1;
        }

}

/* remove the semd having ID=key from the LIST pointed by semd_f: 1 if success, 0 otherwise */
HIDDEN int removefromLIST(int *key, semd_t *semd_f)
{
     if(semd_f==NULL)
     {
		return 0;
     }
     else
     {    
					if(semd_f->s_next!=NULL && (semd_f->s_next)->s_key==key)  /* if semd->s_next has ID=key */
						{
						 semd_t* temp = semd_f->s_next;
						 semd_f->s_next=(semd_f->s_next)->s_next;
						 temp->s_next = NULL;
						 return 1;                          
						}
				    else
						{	return removefromLIST(key, semd_f->s_next);	}
	}
}

/* auxiliary function for iniASL */
HIDDEN void STabletoList(int i)
{
if (i==MAXPROC-1)
	{
     	semd_table[i].s_next=NULL;return;
	}
else
	{
     	semd_table[i].s_next = &(semd_table[i+1]);
     	i=i+1;
     	STabletoList(i);
	}    
}


/**************OFFICIAL FUNCTIONS****************/

/*  funzione dubbia boh */

semd_t* getSemd(int *key){
	return find(key, semd_h);
}

/* inserts p in the queue of the semaphore having ID=&key
returns 1 if semd with ID=key isn't in ASL and semdFree is empty, 0 otherwise  */
int insertBlocked(int *key,pcb_t *p)
{
  pcb_t **headq;
  semd_t *fnd = find(key, semd_h);
  if (fnd==NULL)
  {
      int r=FreetoASL(key); /* insert a new semd from free list to ASL */
      if (r==0)
      {
      return 1; /* return 1 if free list is empty */
      }
      else
      {
      fnd=semd_h;
      headq=&(fnd->s_procQ);
      insertProcQ(headq, p);
      p->p_semkey = key;
      return 0;
      }
   }
   else
   {
       headq=&(fnd->s_procQ);
       insertProcQ(headq, p);
       return 0;
   }  
    
}

/*removes the first pcb in the queue of the semaphore having ID=&key
returns the pcb removed, NULL if semd with  ID=key isn't in ASL
if the queue becomes empty, it moves the semd from ASL to semdFree  */
pcb_t* removeBlocked(int *key)
{
     semd_t *fnd = find(key, semd_h);
     if (fnd==NULL)
     {
         return NULL;
     }
     else
     {
        pcb_t **headq = &(fnd->s_procQ);
        pcb_t* t = removeProcQ(headq);
        if(fnd->s_procQ==NULL)
        {
        semd_t* temp=semdFree_h;
        if (semd_h->s_key==key)  	/* if the semd with ID=key is the first of the list */
		{
			semd_t* temp = semd_h;
			semd_h = semd_h->s_next;
			temp->s_next = NULL;
		}
		else {removefromLIST(key, semd_h);}
        semdFree_h=fnd;
        fnd->s_next=temp;
        t->p_semkey=NULL;
        return t;
        }
        else
        {
		t->p_semkey=NULL;
        return t;
        } 
     }
}

/* procedure that calls fun function for every pcb in s_proQ of the semd with ID=key */
void forallBlocked(int *key, void fun(struct pcb_t *pcb, void *), void *arg)
{
     semd_t* fnd = find(key, semd_h);
     if(fnd==NULL)
     {
     return;
     }
     else
     {
     pcb_t* head = fnd->s_procQ;
     forallProcQ(head, (*fun), arg);
     }
}
 

/* initializes semdFree_h with semd_table[] elements */
void initASL()
{
 /*    STabletoList(0, semdFree_h);*/
    semdFree_h = &(semd_table[0]);
    STabletoList(0);
}
  
/* returns the pointer of the first blocked process on the semd with ID=key, without deallocating it */

pcb_t* headBlocked(int *key){
	semd_t* fnd = find(key, semd_h);
	   if(fnd==NULL)
     {
     return NULL;
     }
     else
     {
	return headProcQ(fnd->s_procQ);
     } 
}


HIDDEN void outSibling(pcb_t **h, pcb_t *p); /* prototype of the function being called in the outChildBlocked */
/* removes p in the queue where it's blocked then removes also the discendents of p */
void outChildBlocked(pcb_t *p)
{
     if(p==NULL)
     { 
        return;
     }
     else
     {
       int *key=p->p_semkey;
       semd_t* fnd= find(key, semd_h);
       pcb_t** headq = &(fnd->s_procQ);
       outProcQ(headq, p);  /* uses the corrispondent function of the pcb */
       outSibling(headq, p->p_first_child); /* deletes the discendents */
     }
}

/* FUNZIONE AUSILIARIA DA AGGIUNGERE SOPRA */
HIDDEN void outSibling(pcb_t **h, pcb_t *p)
{
       if (p==NULL)
       {
          return;
       }
       else
       {
          outChildBlocked(p);
          outSibling(h, p->p_sib);
       }
}
