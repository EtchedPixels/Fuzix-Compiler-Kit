all: lib8070.a crt0.o

OBJ = __bool.o __cc.o __castc_.o __cast_l.o \
      __cceq.o __ccgt.o __ccgteq.o __cclt.o __cclteq.o \
      __shl.o __shr.o __shleq.o __shreq.o __shleqc.o __shrequc.o \
      __div.o __remu.o __muleq.o __mpyfix.o \
      __pluseqc.o __minuseqc.o \
      __minusl.o __plusl.o \
      __switch.o __switchc.o \
      __shrequ.o __shrequc.o \
      __callea.o \
      __cceql.o __ccnel.o __ccltul.o __ccltequl.o __ccgtul.o __ccgtequl.o \
      __ccltl.o __cclteql.o __ccgtl.o __ccgteql.o \
      __booll.o __cpll.o \
      __switchl.o \
      __pluseql.o __minuseql.o __minusminusl.o __plusplusl.o \
      __andl.o __orl.o __xorl.o \
      __andeql.o __oreql.o __xoreql.o \
      __shll.o __shrul.o \
      __mull.o __div32x32.o __divul.o __divl.o \
      __boolf.o __cast2f.o __castf.o __cceqf.o __ccgteqf.o __ccgtf.o __cclteqf.o \
      __ccltf.o __ccnef.o __divf.o __minusf.o __mulf.o __plusf.o __eqopf.o \
      __optsize.o

.s.o:
	fcc -m8070 -c $<

.c.o:
	fcc -m8070 -O -c $<

lib8070.a: $(OBJ)
	rm -f lib8070.a
	ar qc lib8070.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a *~
