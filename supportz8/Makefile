all: libz8.a crt0.o

OBJ =	__cast.o __nref.o __nrefl.o __nref12.o __lstref.o __cleanup.o \
	__plusplus.o __plusplusl.o __lplusplus.o __lplusplusl.o \
	__div16x16.o __div32x32.o __divequc.o __divequ.o __div.o \
	__jmpr2.o __switch.o __switchc.o __switchl.o __bool.o __local.o \
	__not.o \
	__muleqc.o __muleq.o __mull.o __mul.o \
	__remequc.o __remequ.o \
	__plusl.o __minusl.o \
	__ccltu.o __ccgtu.o __ccgtequ.o __ccltequ.o \
	__cclt.o __ccgt.o __ccgteq.o __cclteq.o \
	__cceq.o __ccne.o \
	__cceql.o __ccnel.o __ccltl.o __ccgtl.o __cclteql.o __ccgteql.o \
	__cceqconstl.o __ccneconstl.o \
	__ccgtconstl.o __ccltconstl.o \
	__pluseql.o __postincl.o __minuseql.o __postdecl.o \
	__pluseq.o __minuseq.o __pluseqc.o __minuseqc.o \
      __cast2f.o __castf.o __cceqf.o __ccgteqf.o __ccgtf.o __cclteqf.o \
      __ccltf.o __ccnef.o __divf.o __minusf.o __mulf.o __plusf.o \
      __cpluseq.o __cminuseq.o __cpluseql.o __cminuseql.o \
      __argpush.o

.s.o:
	fcc -mz8 -c $<
.c.o:
	fcc -mz8 -O -c $<

libz8.a: $(OBJ)
	rm -f libz8.a
	ar qc libz8.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a *~ makeops
