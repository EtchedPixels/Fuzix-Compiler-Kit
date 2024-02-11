all: libz8.a crt0.o

OBJ =	__cast.o \
	__div16x16.o __div32x32.o __divequc.o __divequ.o __div.o \
	__jmpr2.o __switch.o __switchc.o __bool.o __local.o \
	__muleqc.o __muleq.o __mull.o __mul.o \
	__remequc.o __remequ.o \
	__ccltu.o __ccgtu.o __ccgtequ.o __ccltequ.o \
	__cclt.o __ccgt.o __ccgteq.o __cclteq.o \
	__cceq.o __ccne.o \
	__cceql.o __ccnel.o

.s.o:
	fcc -mz8 -c $<
.c.o:
	fcc -mz8 -O -c $<

libz8.a: $(OBJ)
	rm -f libz8.a
	ar qc libz8.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a *~ makeops
