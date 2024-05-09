all: libnova3.a crt0.o

OBJ = __const.o \
      __coreop.o \
      __byte.o \
      __ccu.o \
      __divu.o __mul.o \
      __minuseq.o __pluseq.o \
      __postdec.o __postinc.o \
      __cpll.o __negl.o \
      __minusl.o __plusl.o __pluseql.o

.s.o:
	fcc -mnova3 -c $<
.c.o:
	fcc -mnova3 -O -c $<

libnova3.a: $(OBJ)
	rm -f libnova3.a
	ar qc libnova3.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a *~ makeops
