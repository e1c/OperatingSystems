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

/****************AUXILIARY FUNCTIONS****************/

/* auxiliary function for find the right semd_t having ID=key from list semd_f: returns the semd_t found, NULL if not found */
HIDDEN semd_t* find(int *key, semd_t *semd_f)
{
        if(semd_f==NULL)   /* if the list is empty */
        {
            return NULL;
        }
            else
           {
        if(semd_f->s_key==key) /* checks if ID=key */
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
        if (semdFree_h==NULL) /* if free list is empty */
        {
        return 0;
        }
        else
        {
        semd_t* temp=semd_h;   /* moves the first semd from the free list to the head of ASL */
        semd_h=semdFree_h;
        semdFree_h=semdFree_h->s_next;
        semd_h->s_next=temp;
        semd_h->s_key=key;  /* set the right ID */
        return 1;
        }

}

/* removes the semd having ID=key from the LIST pointed by semd_f: 1 if success, 0 otherwise */
HIDDEN int removefromLIST(int *key, semd_t *semd_f)
{
     if(semd_f==NULL)  /* if the list is empty */
     {
		return 0;
     }
     else
     {    
		if(semd_f->s_next!=NULL && (semd_f->s_next)->s_key==key)  /* if semd->s_next has ID=key */
			{
				 semd_t* temp = semd_f->s_next; /* the previous semd will point to the next semd of semd_f */
				 semd_f->s_next=(semd_f->s_next)->s_next;
				 temp->s_next = NULL;
				 return 1;                          
			}
	   else
			{	
				return removefromLIST(key, semd_f->s_next);	
			}
	}
}

/* auxiliary function for initASL to initialize the free list */
HIDDEN void STabletoList(int i)
{
if (i==MAXPROC-1)
	{
     	semd_table[i].s_next=NULL;
     	return;
	}
else                                               /* each semd of semd_table will point to the next */
	{
     	semd_table[i].s_next = &(semd_table[i+1]);
     	i=i+1;
     	STabletoList(i);
	}    
}

/* auxiliary function for outChildBlocked */
HIDDEN void outSibling(pcb_t **h, pcb_t *p)
{
       if (p==NULL)  /* if p is NULL */
       {
          return;
       }
       else
       {
          outChildBlocked(p); /* calls outChildBlocked for all the sibling of p */
          outSibling(h, p->p_sib);
       }
}

/* moves a semd from ASL to the free list */
HIDDEN void ASLtoFree(int* key, semd_t* fnd) {
	semd_t* temp=semdFree_h;
        if (semd_h->s_key==key)  	/* if the semd with ID=key is the first of ASL */
		{                           /* moves the first semd from ASL to the head of free list */
			semd_t* temp = semd_h; 
			semd_h = semd_h->s_next;
			temp->s_next = NULL;
		}
		else {
			removefromLIST(key, semd_h); /* else removes the from an arbitrary position */
			}                            /* and puts it in the free list */
        semdFree_h=fnd;
        fnd->s_next=temp;
}

/**************OFFICIAL FUNCTIONS****************/

/*  finds the semd with ID=key from ASL */
semd_t* getSemd(int *key){
	return find(key, semd_h);
}

/* inserts p in the queue of the semaphore having ID=&key
returns 1 if semd with ID=key isn't in ASL and semdFree is empty, 0 otherwise  */
int insertBlocked(int *key,pcb_t *p)
{
  pcb_t **headq;
  semd_t *fnd = getSemd(key);
  if (fnd==NULL)  /* if semd isn't found in ASL, moves a semd from free list to ASL */
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
      insertProcQ(headq, p);  /* uses the corrispondent insertProcQ */
      p->p_semkey = key;  /* p will point to the semd where it's blocked */
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
     semd_t *fnd = getSemd(key);
     if (fnd==NULL) /* if semd is not found */
     {
         return NULL;
     }
     else
     {
        pcb_t **headq = &(fnd->s_procQ);
        pcb_t* t = removeProcQ(headq);
        if(fnd->s_procQ==NULL) /* if the block list becomes empty */
        {
        ASLtoFree(key,fnd);  /* the semd becomes free */
        t->p_semkey=NULL;   /* the pcb will not point more to the semd */
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
     semd_t* fnd = getSemd(key);  /* find the right semd from ASL */
     if(fnd==NULL)
     {
     return;
     }
     else
     {
     pcb_t* head = fnd->s_procQ;
     forallProcQ(head, (*fun), arg);  /* calls the corrispondent function forallProcQ using s_procQ of the semd */
     }
}
 

/* initializes semdFree_h with semd_table[] elements */
void initASL()
{
    semdFree_h = &(semd_table[0]); /* the first element of semd_table will be the head of the free list */
    STabletoList(0);               /* then links the following semds */
}
  
/* returns the pointer of the first blocked process on the semd with ID=key, without deallocating it */
pcb_t* headBlocked(int *key){
	semd_t* fnd = getSemd(key);
	   if(fnd==NULL)  /* if the semd with ID=key isn't found */
     {
     return NULL;
     }
     else
     {
	return headProcQ(fnd->s_procQ);  /* calls the corrispondent function headProcQ using s_procQ of the semd */
     } 
}

/* removes p in the block list of the semd where it's blocked */
pcb_t* outBlocked(pcb_t *p)
{
     if(p==NULL)
     { 
        return NULL;
     }
     else
     {
       int *key=p->p_semkey;  /* find the semd where p is blocked */
       semd_t* fnd= getSemd(key);
       pcb_t** headq = &(fnd->s_procQ);
       pcb_t* t=outProcQ(headq, p);  /* uses the corrispondent function of the pcb */
       if(fnd->s_procQ==NULL)     /* checks if the block list becomes empty */
        {
        ASLtoFree(key,fnd);  /* the semd becomes free */
        t->p_semkey=NULL;    /* the pcb will not point more to the semd */
        }
        else
        {
		t->p_semkey=NULL;
        }
        return t;
     }
}

/* removes p in the queue where it's blocked then removes also the discendents of p which are blocked */

/* PHASE 2 EDIT: aggiunto il campo terminated che viene modificato*/
void outChildBlocked(pcb_t *p)
{
     if(p==NULL)
     { 
        return;
     }
     else
     {
       int *key=p->p_semkey;  /* find the semd where p is blocked */
       semd_t* fnd= getSemd(key);
       pcb_t** headq = &(fnd->s_procQ);
       pcb_t* t=outProcQ(headq, p);  /* uses the corrispondent function of the pcb */
       if(fnd->s_procQ==NULL)  /* checks if the block list becomes empty */
        {
        ASLtoFree(key,fnd);  /* the semd becomes free */
        t->p_semkey=NULL;    /* the pcb will not point more to the semd */
        }
        else
        {
		t->p_semkey=NULL;
		
        }
		t->terminated=TRUE;
		outSibling(headq, p->p_first_child); /* deletes the discendents */
     }
}

