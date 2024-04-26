all: lib6809.a crt0.o

OBJ =  dp.o makebool.o divide.o __div.o __divu.o __xdiveq.o __mul.o \
       __shl.o __shr.o __shru.o \
       __minus.o __xminuseq.o \
       __cceql.o __ccnel.o __ccgtl.o __ccgteql.o __ccltl.o __cclteql.o \
       __switch.o __switchc.o __switchl.o \
       __cpll.o __castl.o __negatel.o __booll.o __notl.o \
       __xplusplusuc.o __xshleq.o __xshreq.o __xshrequ.o \
       __xshleqc.o __xshreqc.o __xshrequc.o \
      __xminusminusl.o __xminuseql.o __xplusplusl.o __xpluseql.o \
      __xminusminus.o __xminusminusc.o __xmuleq.o __xmuleql.o \
      __shll.o __shrl.o __shrul.o \
      __xshleql.o __xshreql.o __xshrequl.o \
      __minusl.o __mull.o divide32x32.o __divul.o __remul.o __divl.o __reml.o \
      __xdivequl.o __xremequl.o __xdiveql.o __xremeql.o __xdiveqc.o __xdivequc.o \
      __cast2f.o __castf.o __cceqf.o __ccgteqf.o __ccgtf.o __cclteqf.o \
      __ccltf.o __ccnef.o __divf.o __minusf.o __mulf.o __plusf.o
    
.s.o:
	fcc -m6809 -c $<
.c.o:
	fcc -m6809 -O -c $<

lib6809.a: $(OBJ)
	rm -f lib6809.a
	ar qc lib6809.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a *~ makeops
