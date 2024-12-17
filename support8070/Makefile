all: lib8070.a crt0.o

OBJ = __bool.o __cc.o __castc_.o \
      __cceq.o __ccne.o __ccgt.o __ccgteq.o __cclt.o __cclteq.o \
      __ccgtu.o __ccgtequ.o __ccltu.o __ccltequ.o \
      __minusl.o __plusl.o \
      __switch.o __switchc.o \
      __shl_t.o __shr_t.o __sub_t.o

.s.o:
	fcc -m8070 -c $<

.c.o:
	fcc -m8070 -O -c $<

lib8070.a: $(OBJ)
	rm -f lib8070.a
	ar qc lib8070.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a *~
