all: libnova.a crt0.o

OBJ = __dp.o __const.o\
      __coreop.o \
      __byte.o \
      __ccu.o \
      __divu.o __mul.o \
      __minuseq.o __pluseq.o \
      __postdec.o __postinc.o \
      __cpll.o __negl.o \
      __minusl.o __plusl.o __minuseql.o __pluseql.o \
      __shift.o __logiceq.o

.s.o:
	fcc -mnova3 -c $<
.c.o:
	fcc -mnova3 -O -c $<

libnova.a: $(OBJ)
	rm -f libnova.a
	ar qc libnova.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a *~ makeops
