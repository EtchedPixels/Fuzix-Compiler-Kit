all: lib6809.a crt0.o

OBJ =  dp.o makebool.o divide.o __div.o __divu.o __xdiveq.o __mul.o \
       __shl.o __shr.o __shru.o \
       __xminuseq.o \
       __cceql.o __ccnel.o __ccgtl.o __ccgteql.o __ccltl.o __cclteql.o \
       __switch.o __switchc.o \
       __cpll.o __castl.o __negatel.o  \
       __xplusplusuc.o
    
.s.o:
	fcc -m6809 -c $<
.c.o:
	fcc -m6809 -O -c $<

lib6809.a: $(OBJ)
	rm -f lib6809.a
	ar qc lib6809.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a *~ makeops
