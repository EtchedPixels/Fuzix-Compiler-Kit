all: lib6803.a crt0.o

OBJ =  dp.o makebool.o divide.o __div.o __divu.o __xdiveq.o __mul.o \
       __shl.o __shr.o __shru.o \
       __minus.o __xminuseq.o \
       __cceql.o __ccnel.o __ccgtl.o __ccgteql.o __ccltl.o __cclteql.o \
       __switch.o __switchc.o \
       __cpll.o __castl.o __negatel.o __booll.o __notl.o \
       __xplusplusuc.o __xshleq.o __xshreq.o __xshrequ.o \
       __xshleqc.o __xshreqc.o __xshrequc.o \
      __xminuseql.o __xplusplusl.o __xpluseql.o \
      __xminusminus.o __xminusminusc.o __xmuleq.o __xmuleql.o \
      __shll.o __shrl.o __shrul.o \
      __xshleql.o __xshreql.o __xshrequl.o \
      __minusl.o __mull.o divide32x32.o __divul.o __remul.o __divl.o __reml.o \
      __xdiveqc.o __xdivequc.o \
      __cast2f.o __castf.o __cceqf.o __ccgteqf.o __ccgtf.o __cclteqf.o \
      __ccltf.o __ccnef.o __divf.o __minusf.o __mulf.o __plusf.o \
      __xminusminusl.o __xdivequl.o __xremequl.o __xdiveql.o __switchl.o \
      __addxconst.o __xremeql.o __castc.o

.s.o:
	fcc -m6803 -c $<
.c.o:
	fcc -m6803 -O -c $<

lib6803.a: $(OBJ)
	rm -f lib6803.a
	ar qc lib6803.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a *~ makeops
