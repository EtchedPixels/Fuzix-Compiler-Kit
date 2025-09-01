all: libnova.a crt0.o

OBJ = __dp.o __const.o\
      __coreop.o \
      __byte.o \
      __cast.o \
      __cc.o \
      __div.o __mul.o \
      __minuseq.o __pluseq.o \
      __postdec.o __postinc.o \
      __ccl.o __cpll.o __negl.o \
      __minusl.o __plusl.o __minuseql.o __pluseql.o \
      __divul.o __mull.o \
      __shift.o __shifteq.o __logiceq.o __logiceql.o \
      __switch.o \
      __boolf.o __cast2f.o __castf.o __cceqf.o __ccgteqf.o __ccgtf.o __cclteqf.o \
      __ccltf.o __ccnef.o __divf.o __minusf.o __mulf.o __plusf.o __eqopf.o

.s.o:
	fcc -mnova3 -c $<
.c.o:
	fcc -mnova3 -O -c $<

libnova.a: $(OBJ)
	# TODO: debug why this extra is needed
	rm -f libnova.a
	ar qc libnova.a `../lordernova $(OBJ) | tsort` __byte.o

clean:
	rm -f *.o *.a *~ makeops
