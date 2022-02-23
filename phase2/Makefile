# Created by: Cattani Giovanni, Frascolla Stefano, Giorgi Riccardo, Engjell Rrapaj
# Mikefile SOS phase1

# Commands:
PARAM = -ansi -pedantic -Wall -c
COMPGCC = mipsel-linux-gcc
COMPLD = mipsel-linux-ld -T
PATH1 = /usr/share/umps2/elf32ltsmip.h.umpscore.x
PATH2 = /usr/lib/umps2/crtso.o
PATH3 = /usr/lib/umps2/libumps.o 


output: kernel
		umps2-elf2umps -k kernel

kernel: pcb.o asl.o p1test.o
		$(COMPLD) $(PATH1) $(PATH2) p1test.o asl.o pcb.o $(PATH3) -o kernel

pcb.o: pcb.c
		$(COMPGCC) $(PARAM) pcb.c 

asl.o: asl.c
		$(COMPGCC) $(PARAM) asl.c 

p1test.o: p1test.c	
		$(COMPGCC) $(PARAM) p1test.c
