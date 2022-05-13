all: lib8085.a crt0.o

OBJ = workspace.o \
      __and.o __andeq.o __or.o __oreq.o __xor.o __xoreq.o \
      __minuseq.o __pluseq.o \
      __bool.o __cpl.o __not.o \
      __boolc.o __cplc.o __notc.o \
      __minus.o __plus.o \
      __cclt.o __ccltu.o __ccgteq.o __ccgtequ.o \
      __ccgt.o __ccgtu.o __cclteq.o __ccltequ.o \
      __andc.o __orc.o __xorc.o \
      __minuseqc.o __pluseqc.o \
      __castc.o __castcl.o

.s.o:
	cc85 -c $<

lib8085.a: $(OBJ)
	rm -f lib8085.a
	ar qc lib8085.a $(OBJ)

clean:
	rm *.o *.a
