all: lib6803.a crt0.o

OBJ =  dp.o makebool.o
    
.s.o:
	fcc -m6803 -c $<
.c.o:
	fcc -m6803 -O -c $<

lib6803.a: $(OBJ)
	rm -f lib6803.a
	ar qc lib6803.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a *~ makeops
