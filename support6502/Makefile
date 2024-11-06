all: lib6502.a crt0.o

OBJ =  __addysp.o __addyasp.o __subysp.o __asp.o __yasp.o __lxaysptmp.o __lxa0sptmp.o \
       __gloy.o __gloyc.o \
       __minuseqtmp.o __minus.o \
       __plusplus1.o __plusplus2.o __plusplus.o __plusplusy.o __plus.o \
       __poptmp.o __pushc.o __pushl.o __push.o \
       zeropage.o \
       __bool.o __not.o __eqeqtmp.o \
       __lttmp.o __lttmpu.o __gteqtmp.o  __gteqtmpu.o \
       __lstmp.o __rstmp.o

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
	ar qc lib6502.a $(OBJ) $(GEN)

clean:
	rm -f *.o *.a  $(GEN) *~ makeops
