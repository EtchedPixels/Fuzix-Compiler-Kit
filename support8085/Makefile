all: lib8085.a crt0.o

OBJ = workspace.o __true.o __switchc.o \
      __and.o __andeq.o __or.o __oreq.o __xor.o __xoreq.o \
      __minuseq.o __pluseq.o \
      __bool.o __cpl.o __not.o \
      __boolc.o __cplc.o __notc.o \
      __minus.o __plus.o \
      __postinc.o __postdec.o \
      __shl.o __shr.o \
      __divdeu.o \
      __cclt.o __ccltu.o __ccgteq.o __ccgtequ.o \
      __cceq.o __ccne.o __cmpeq.o __cmpne.o \
      __ccgt.o __ccgtu.o __cclteq.o __ccltequ.o __cmpeq0.o \
      __cmpgtu.o __cmpltu.o __cmpltequ.o __cmpgtequ.o \
      __cmpgt.o __cmplt.o __cmplteq.o __cmpgteq.o \
      __cmpgt0.o __cmplteq0.o __cmpgteq0.o __cmplt0.o \
      __andc.o __orc.o __xorc.o __andeqc.o __oreqc.o __xoreqc.o \
      __minuseqc.o __pluseqc.o \
      __castc.o __castcl.o __cast_l.o __cast_ul.o \
      __postincc.o __postdecc.o \
      __bandl.o __borl.o __bxorl.o \
      __booll.o \
      __assignl.o \
      __bandde.o __borde.o __bxorde.o __mulde.o __shrdeu.o \
      __muldec.o

.s.o:
	cc85 -c $<

lib8085.a: $(OBJ)
	rm -f lib8085.a
	ar qc lib8085.a $(OBJ)

clean:
	rm -f *.o *.a
