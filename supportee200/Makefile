all: libee200.a crt0.o

OBJ =  dp.o __bool.o __mul.o div16x16.o __divu.o __div.o \
	__muleq.o __diveq.o __remeq.o __divequ.o __remequ.o \
       __shl.o __shr.o __shln.o __shrn.o __shrun.o \
       __shleq.o __shreq.o __shrequ.o \
       __shleqc.o __shreqc.o __shrequc.o \
       __switch.o __switchc.o __switchl.o \
       __booll.o __cpll.o __negatel.o \
       __bandl.o __orl.o __xorl.o \
       __cceql.o __ccnel.o __ccltl.o __ccgteql.o __ccgtl.o __cclteql.o \
       __plusl.o __minusl.o __pluseql.o __minuseql.o \
       __plusplusl.o __minusminusl.o \
       __andeql.o __oreql.o __xoreql.o \
       __mull.o __muleql.o \
       div32x32.o __divul.o __divl.o __diveql.o __divequl.o \
       __shll.o __shrl.o __shrul.o __shleql.o __shreql.o __shrequl.o \
       __castc_.o __cast_l.o
    
.s.o:
	fcc -mee200 -c $<
.c.o:
	fcc -mee200 -O -c $<

libee200.a: $(OBJ)
	rm -f libee200.a
	ar qc libee200.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a *~ makeops
