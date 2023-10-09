all: lib65c816.a crt0.o

OBJ = __andeqx.o __diveqx.o __diveqxu.o __divx.o __eqeqx.o __gteqx.o \
      __gteqxu.o __lsx.o __ltx.o __ltxu.o __minuseqx.o __muleqx.o __mulx.o \
      __nex.o __oreqx.o __pluseqx.o __remeqx.o __remeqxu.o __rsx.o __rsxu.o \
      __shleqx.o __shreqx.o __shreqxu.o __xoreqx.o __gtx.o __gtxu.o \
      __lteqx.o __lteqxu.o \
      __bool.o __not.o \
      __andeqxc.o __diveqxc.o __minuseqxc.o __muleqxc.o __oreqxc.o \
      __pluseqxc.o __remeqxc.o __remeqxcu.o __shleqxc.o __shreqxc.o \
      __shreqxcu.o __diveqxcu.o \
      __plusl.o __minusl.o __booll.o __notl.o \
      __shll.o __cast_l.o __negatel.o __longcmp.o \
      __divul.o __divl.o \
      dp.o __push0.o __fnexit.o \
      __switchc.o __switch.o __switchl.o \
      __div32x32.o __mull.o
    
.s.o:
	cc65c816 -c $<

lib65c816.a: $(OBJ)
	rm -f lib65c816.a
	ar qc lib65c816.a $(OBJ) $(GEN)

clean:
	rm -f *.o *.a *~ makeops
