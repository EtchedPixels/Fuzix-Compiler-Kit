all: lib65c816.a crt0.o

# This currently is in a working sort order but needs fixing to use lorder
# properly
OBJ = __cast2f.o __castf.o __cceqf.o __ccgteqf.o __ccgtf.o __cclteqf.o \
      __ccltf.o __ccnef.o __divf.o __minusf.o __mulf.o __plusf.o \
      __andeqx.o __diveqx.o __diveqxu.o __divx.o __eqeqx.o __gteqx.o \
      __gteqxu.o __lsx.o __ltx.o __ltxu.o __minuseqx.o __muleqx.o __mulx.o \
      __nex.o __oreqx.o __pluseqx.o __remeqx.o __remeqxu.o __rsx.o __rsxu.o \
      __shleqx.o __shreqx.o __shreqxu.o __xoreqx.o __gtx.o __gtxu.o \
      __lteqx.o __lteqxu.o __muln.o \
      __bool.o __not.o \
      __andeqxc.o __diveqxc.o __minuseqxc.o __muleqxc.o __oreqxc.o \
      __pluseqxc.o __remeqxc.o __remeqxuc.o __shleqxc.o __shreqxc.o \
      __shreqxuc.o __diveqxuc.o \
      __plusl.o __minusl.o __booll.o __notl.o \
      __shll.o __cast_l.o __negatel.o __longcmp.o \
      __divul.o __divl.o __pluseqxl.o __minuseqxl.o __ltxl.o __gtxul.o \
      __bandl.o __orl.o __xorl.o __cpll.o \
      __shrxl.o __shlxl.o \
      __postdecl.o __postincl.o  __postdecxl.o __postincxl.o \
      __push0.o __fnexit.o \
      __switchc.o __switch.o __switchl.o \
      __div32x32.o __mull.o \
      _memcpy.o _memset.o _strlen.o \
      __adceql.o __andeql.o __oreql.o __eoreql.o \
      dp.o
    
.s.o:
	fcc -m65c816 -c $<
.c.o:
	fcc -m65c816 -O -c $<

lib65c816.a: $(OBJ)
	rm -f lib65c816.a
	ar qc lib65c816.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a *~ makeops
