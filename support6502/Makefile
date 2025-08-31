all: lib6502.a crt0.o

OBJ =  __addysp.o __addyasp.o __subysp.o __asp.o __yasp.o __lxaysptmp.o \
       __lxa0sptmp.o __gloy.o __gloyl.o __lstxay.o \
       __minuseqtmp.o __minus.o __divu.o __mulu.o __div.o \
       __plusplus1.o __plusplus2.o __plusplus.o __plusplusy.o __plus.o \
       __pluseqy.o __pluseq.o \
       __poptmp.o __pushc.o __pushl.o __push.o __ytmp.o __ytmpc.o \
       __callfunc.o __switchc.o __switch.o __switchl.o \
       zeropage.o __eqop.o \
       __bool.o __boolc.o __not.o __notc.o __eqeqtmp.o __netmp.o \
       __eqeqtmpc.o __netmpc.o \
       __lttmp.o __lttmpu.o __gteqtmp.o  __gteqtmpu.o \
       __lttmpc.o __lttmpuc.o __gteqtmpc.o __gteqtmpuc.o \
       __lstmp.o __rstmp.o \
       __shrequc.o __shrequ.o \
       __cpl.o \
       __cast_l.o __booll.o __notl.o __derefl.o __assignl.o __cpll.o \
       __pop32.o __bandl.o __orl.o __xorl.o __lcmp.o __negatel.o \
       __pluseql.o __postincl.o __minuseql.o \
       __andeql.o __oreql.o __xoreql.o

GEN =  __adcspy.o __adctmpy.o __sbcspy.o __sbctmpy.o __andspy.o __andtmpy.o __eorspy.o __eortmpy.o \
       __ldaspy.o __ldatmpy.o __oraspy.o __oratmpy.o __staspy.o __statmpy.o \
       __adctmp.o __sbctmp.o __andtmp.o __oratmp.o __eortmp.o \
       __adceqtmp.o __andeqtmp.o __oraeqtmp.o __eoreqtmp.o
    
makeops: makeops.c
	$(CC) makeops.c -o makeops

.s.o:
	fcc -m6502 -c $<

__adcspy.s: makeops
	./makeops

lib6502.a: $(OBJ) $(GEN)
	rm -f lib6502.a
	ar qc lib6502.a `../lorderz80 $(OBJ) $(GEN) | tsort`

clean:
	rm -f *.o *.a  $(GEN) *~ makeops
