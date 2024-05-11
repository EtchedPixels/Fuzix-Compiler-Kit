all: cc cc0 \
     cc1.8080 cc1.z80 cc1.thread cc1.byte cc1.6502 \
     cc1.65c816 cc1.z8 cc1.1802 cc1.6800 cc1.6809 \
     cc1.8070 cc1.8086 \
     cc1.ee200 cc1.nova \
     cc2 cc2.8080 cc2.z80 cc2.65c816 cc2.thread \
     cc2.6502 cc2.z8 cc2.super8 cc2.1802 cc2.6800 cc2.6809 \
     cc2.8070 cc2.8086 \
     cc2.ee200 cc2.nova \
     copt \
     support6303 support6502 support65c816 support6800 support6803 \
     support6809 support68hc11 support8080 support8085 supportz80 \
     supportz8 supportsuper8 supportee200 supportnova3 test

bootstuff: cc cc0 \
     cc1.8080 cc1.z80 cc1.thread cc1.byte cc1.6502 \
     cc1.65c816 cc1.z8 cc1.super8 cc1.1802 cc1.6800 cc1.6809 \
     cc1.8070 cc1.8086 cc1.ee200 cc1.nova \
     cc2 cc2.8080 cc2.z80 cc2.65c816 cc2.thread \
     cc2.6502 cc2.z8 cc2.super8 cc2.1802 cc2.6800 cc2.6809 \
     cc2.8070 cc2.8086 cc2.ee200 cc2.nova \
     copt

.PHONY: support6303 support6502 support65c816 support6800 support6803 \
	support6809 support68hc11 support8080 support8085 supportsuper8 \
	supportz8 supportz80 supportee200 supportnova3 \
	test

CCROOT ?=/opt/fcc/

OBJS0 = frontend.o

OBJS1 = body.o declaration.o enum.o error.o expression.o header.o idxdata.o \
	initializer.o label.o lex.o main.o primary.o stackframe.o storage.o \
	struct.o switch.o symbol.o tree.o type.o type_iterator.o

OBJS2 = backend.o backend-default.o
OBJS3 = backend.o backend-8080.o
OBJS4 = backend.o backend-8086.o
OBJS5 = backend.o be-codegen-z80.o be-rewrite-z80.o be-func-z80.o
OBJS6 = backend.o backend-65c816.o
OBJS7 = backend.o backend-ee200.o
OBJS8 = backend.o backend-8070.o
OBJS9 = backend.o backend-threadcode.o
OBJS10 = backend.o backend-nova.o
OBJS11 = backend.o backend-6502.o
OBJS12 = backend.o backend-65c816.o
OBJS13 = backend.o backend-z8.o
OBJS14 = backend.o backend-super8.o
OBJS15 = backend.o backend-1802.o
OBJS16 = backend.o be-codegen-6800.o be-track-6800.o be-code-6800.o be-func-6800.o
OBJS17 = backend.o be-codegen-6800.o be-track-6800.o be-code-6809.o be-func-6800.o

CFLAGS = -Wall -pedantic -g3 -DLIBPATH="\"$(CCROOT)/lib\"" -DBINPATH="\"$(CCROOT)/bin\""

INC0 = token.h
INC1 = body.h compiler.h declaration.h enum.h error.h expression.h header.h \
       idxdata.h initializer.h label.h lex.h primary.h stackframe.h storage.h \
       struct.h symbol.h target.h token.h tree.h type.h type_iterator.h
INC2 = backend.h symtab.h

$(OBJS0): $(INC0) symtab.h

$(OBJS1): $(INC1)

$(OBJS2): $(INC1) $(INC2)

$(OBJS3): $(INC1) $(INC2)

backend-super8.o: backend-super8.c backend-z8.c

cc:	cc.o
	gcc -g3 cc.c -o cc

cc0:	$(OBJS0)
	gcc -g3 $(OBJS0) -o cc0

cc1.8080:$(OBJS1) target-8080.o
	gcc -g3 $(OBJS1) target-8080.o -o cc1.8080

cc1.8086:$(OBJS1) target-8086.o
	gcc -g3 $(OBJS1) target-8086.o -o cc1.8086

cc1.z80:$(OBJS1) target-z80.o
	gcc -g3 $(OBJS1) target-z80.o -o cc1.z80

cc1.thread:$(OBJS1) target-threadcode.o
	gcc -g3 $(OBJS1) target-threadcode.o -o cc1.thread

cc1.byte:$(OBJS1) target-bytecode.o
	gcc -g3 $(OBJS1) target-bytecode.o -o cc1.byte

cc1.6502:$(OBJS1) target-6502.o
	gcc -g3 $(OBJS1) target-6502.o -o cc1.6502

cc1.65c816:$(OBJS1) target-65c816.o
	gcc -g3 $(OBJS1) target-65c816.o -o cc1.65c816

cc1.z8:$(OBJS1) target-z8.o
	gcc -g3 $(OBJS1) target-z8.o -o cc1.z8

cc1.super8:$(OBJS1) target-super8.o
	gcc -g3 $(OBJS1) target-super8.o -o cc1.super8

cc1.1802:$(OBJS1) target-1802.o
	gcc -g3 $(OBJS1) target-1802.o -o cc1.1802

cc1.6800:$(OBJS1) target-6800.o
	gcc -g3 $(OBJS1) target-6800.o -o cc1.6800

cc1.6809:$(OBJS1) target-6800.o
	gcc -g3 $(OBJS1) target-6800.o -o cc1.6809

cc1.8070:$(OBJS1) target-8070.o
	gcc -g3 $(OBJS1) target-8070.o -o cc1.8070

cc1.ee200:$(OBJS1) target-ee200.o
	gcc -g3 $(OBJS1) target-ee200.o -o cc1.ee200

cc1.nova:$(OBJS1) target-nova.o
	gcc -g3 $(OBJS1) target-nova.o -o cc1.nova

cc2:	$(OBJS2)
	gcc -g3 $(OBJS2) -o cc2

cc2.8080:	$(OBJS3)
	gcc -g3 $(OBJS3) -o cc2.8080

cc2.8086:	$(OBJS4)
	gcc -g3 $(OBJS4) -o cc2.8086

cc2.z80:	$(OBJS5)
	gcc -g3 $(OBJS5) -o cc2.z80

cc2.thread:	$(OBJS9)
	gcc -g3 $(OBJS9) -o cc2.thread

cc2.6502:	$(OBJS11)
	gcc -g3 $(OBJS11) -o cc2.6502

cc2.65c816:	$(OBJS12)
	gcc -g3 $(OBJS12) -o cc2.65c816

cc2.z8:		$(OBJS13)
	gcc -g3 $(OBJS13) -o cc2.z8

cc2.super8:	$(OBJS14)
	gcc -g3 $(OBJS14) -o cc2.super8

cc2.1802:	$(OBJS15)
	gcc -g3 $(OBJS15) -o cc2.1802

cc2.6800:	$(OBJS16)
	gcc -g3 $(OBJS16) -o cc2.6800

cc2.6809:	$(OBJS17)
	gcc -g3 $(OBJS17) -o cc2.6809

cc2.8070:	$(OBJS8)
	gcc -g3 $(OBJS8) -o cc2.8070

cc2.ee200:	$(OBJS7)
	gcc -g3 $(OBJS7) -o cc2.ee200

cc2.nova:	$(OBJS10)
	gcc -g3 $(OBJS10) -o cc2.nova

support6303:
	(cd support6303; make)

support6502:
	(cd support6502; make)

support65c816:
	(cd support65c816; make)

support6800:
	(cd support6800; make)

support6803:
	(cd support6803; make)

support6809:
	(cd support6809; make)

support68hc11:
	(cd support68hc11; make)

support8080:
	(cd support8080; make)

support8085:
	(cd support8085; make)

supportee200:
	(cd supportee200; make)

supportnova3:
	(cd supportnova3; make)

supportsuper8:
	(cd supportsuper8; make)

supportz8:
	(cd supportz8; make)

supportz80:
	(cd supportz80; make)

test:
	(cd test; make)

clean:
	rm -f cc cc0 copt
	rm -f cc6502 cc65c816
	rm -f cc1.1802 cc2.1802
	rm -f cc1.6800 cc2.6800
	rm -f cc1.6809 cc2.6809
	rm -f cc1.8080 cc1.z80 cc1.thread
	rm -f cc1.6502 cc1.65c816 cc1.byte
	rm -f cc1.8070
	rm -f cc2.8080 cc2.z80 cc2.65c816
	rm -f cc2.8070 cc2.thread cc2.byte cc2.6502
	rm -f cc1.super8 cc2.super8
	rm -f cc1.z8 cc2.z8
	rm -f cc1.8086 cc2.8086
	rm -f cc1.ee200 cc2.ee200
	rm -f *~ *.o
	(cd support6303; make clean)
	(cd support6502; make clean)
	(cd support65c816; make clean)
	(cd support6800; make clean)
	(cd support6803; make clean)
	(cd support6809; make clean)
	(cd support68hc11; make clean)
	(cd support8080; make clean)
	(cd supportee200; make clean)
	(cd supportnova3; make clean)
	(cd supportsuper8; make clean)
	(cd support8085; make clean)
	(cd supportz80; make clean)
	(cd supportz8; make clean)
	(cd test; make clean)

# New install version. This is used by both the install rules, as we need
# to bootstrap build a toolchain with no support library to build the toolchain
# with out.
#
#	Install the tools only
#
bootinst:
	mkdir -p $(CCROOT)/bin
	mkdir -p $(CCROOT)/lib
	cp cc $(CCROOT)/bin/fcc
	cp cc.hlp $(CCROOT)/lib/cc.hlp
	cp cc0 $(CCROOT)/lib
	cp cpp $(CCROOT)/lib
	# 6502
	mkdir -p $(CCROOT)/lib/6502
	mkdir -p $(CCROOT)/lib/6502/include
	cp cc1.6502 $(CCROOT)/lib
	cp cc2.6502 $(CCROOT)/lib
	cp copt $(CCROOT)/lib
	cp rules.6502 $(CCROOT)/lib
	# 65c816
	mkdir -p $(CCROOT)/lib/65c816
	mkdir -p $(CCROOT)/lib/65c816/include
	cp cc1.65c816 $(CCROOT)/lib
	cp cc2.65c816 $(CCROOT)/lib
	cp copt $(CCROOT)/lib
	cp rules.65c816 $(CCROOT)/lib
	# 6800
	mkdir -p $(CCROOT)/lib/6303
	mkdir -p $(CCROOT)/lib/6303/include
	mkdir -p $(CCROOT)/lib/6800
	mkdir -p $(CCROOT)/lib/6800/include
	mkdir -p $(CCROOT)/lib/6803
	mkdir -p $(CCROOT)/lib/6803/include
	mkdir -p $(CCROOT)/lib/6809
	mkdir -p $(CCROOT)/lib/6809/include
	mkdir -p $(CCROOT)/lib/hc11
	mkdir -p $(CCROOT)/lib/hc11/include
	cp cc1.6800 $(CCROOT)/lib
	cp cc2.6800 $(CCROOT)/lib
	cp cc1.6809 $(CCROOT)/lib
	cp cc2.6809 $(CCROOT)/lib
	cp copt $(CCROOT)/lib
	cp rules.6800 $(CCROOT)/lib
	cp rules.6809 $(CCROOT)/lib
	cp rules.hc11 $(CCROOT)/lib
	cp lorder6809 $(CCROOT)/bin/lorder6809
	# 8070 (WIP)
	mkdir -p $(CCROOT)/lib/8070
	mkdir -p $(CCROOT)/lib/8070/include
	cp cc1.8070 $(CCROOT)/lib
	cp cc2.8070 $(CCROOT)/lib
	cp rules.8070 $(CCROOT)/lib
	# 8086 (WIP)
	mkdir -p $(CCROOT)/lib/8086
	mkdir -p $(CCROOT)/lib/8086/include
	cp cc1.8086 $(CCROOT)/lib
	cp cc2.8086 $(CCROOT)/lib
	cp rules.8086 $(CCROOT)/lib
	# 8080/8085
	mkdir -p $(CCROOT)/lib/8080
	mkdir -p $(CCROOT)/lib/8085
	mkdir -p $(CCROOT)/lib/8080/include
	mkdir -p $(CCROOT)/lib/8085/include
	cp lorder8080 $(CCROOT)/bin/lorder8080
	cp cc1.8080 $(CCROOT)/lib
	cp cc2.8080 $(CCROOT)/lib
	cp rules.8080 $(CCROOT)/lib
	cp rules.8085 $(CCROOT)/lib
	# Z80
	mkdir -p $(CCROOT)/lib/z80/
	mkdir -p $(CCROOT)/lib/z80/include
	cp lorderz80 $(CCROOT)/bin/lorderz80
	cp cc1.z80 $(CCROOT)/lib
	cp cc2.z80 $(CCROOT)/lib
	cp rules.z80 $(CCROOT)/lib
	# Threadcode
	mkdir -p $(CCROOT)/lib/threadcode
	mkdir -p $(CCROOT)/lib/threadcode/include
	cp cc1.thread $(CCROOT)/lib
	cp cc2.thread $(CCROOT)/lib
	cp rules.thread $(CCROOT)/lib
	# Z8
	mkdir -p $(CCROOT)/lib/z8
	mkdir -p $(CCROOT)/lib/z8/include/
	cp cc1.z8 $(CCROOT)/lib
	cp cc2.z8 $(CCROOT)/lib
	cp rules.z8 $(CCROOT)/lib
	cp lorderz8 $(CCROOT)/bin/lorderz8
	# Super8
	mkdir -p $(CCROOT)/lib/super8
	mkdir -p $(CCROOT)/lib/super8/include/
	cp cc1.super8 $(CCROOT)/lib
	cp cc2.super8 $(CCROOT)/lib
	cp rules.super8 $(CCROOT)/lib
	cp lorderz8 $(CCROOT)/bin/lordersuper8
	# 1802
	mkdir -p $(CCROOT)/lib/1802
	mkdir -p $(CCROOT)/lib/1802/include/
	cp cc1.1802 $(CCROOT)/lib
	cp cc2.1802 $(CCROOT)/lib
	cp rules.1802 $(CCROOT)/lib
	# ee200
	mkdir -p $(CCROOT)/lib/ee200
	mkdir -p $(CCROOT)/lib/ee200/include/
	cp lorderee200 $(CCROOT)/bin/lorderee200
	cp cc1.ee200 $(CCROOT)/lib
	cp cc2.ee200 $(CCROOT)/lib
	cp rules.ee200 $(CCROOT)/lib
	# Nova
	mkdir -p $(CCROOT)/lib/nova
	mkdir -p $(CCROOT)/lib/nova/include/
	cp lordernova $(CCROOT)/bin/lordernova
	cp cc1.nova $(CCROOT)/lib
	cp cc2.nova $(CCROOT)/lib
	cp rules.nova $(CCROOT)/lib

#
#	Install the support libraries
#
libinst:
	cp support6502/crt0.o $(CCROOT)/lib/6502/
	cp support6502/lib6502.a $(CCROOT)/lib/6502/lib6502.a
#	cp support6502/lib65c02.a $(CCROOT)/lib/6502/lib65c02.a
	ar cq $(CCROOT)/lib/6502/libc.a
	cp support65c816/crt0.o $(CCROOT)/lib/65c816/
	cp support65c816/lib65c816.a $(CCROOT)/lib/65c816/lib65c816.a
	ar cq $(CCROOT)/lib/65c816/libc.a
	cp support6800/crt0.o $(CCROOT)/lib/6303/
	cp support6800/lib6800.a $(CCROOT)/lib/6303/lib6303.a
	ar cq $(CCROOT)/lib/6800/libc.a
	cp support6800/crt0.o $(CCROOT)/lib/6800/
	cp support6800/lib6800.a $(CCROOT)/lib/6800/lib6800.a
	ar cq $(CCROOT)/lib/6800/libc.a
	cp support6803/crt0.o $(CCROOT)/lib/6803/
	cp support6803/lib6803.a $(CCROOT)/lib/6803/lib6803.a
	ar cq $(CCROOT)/lib/6803/libc.a
	cp support6809/crt0.o $(CCROOT)/lib/6809/
	cp support6809/lib6809.a $(CCROOT)/lib/6809/lib6809.a
	ar cq $(CCROOT)/lib/6809/libc.a
	cp support68hc11/crt0.o $(CCROOT)/lib/hc11/
	cp support68hc11/lib68hc11.a $(CCROOT)/lib/hc11/libhc11.a
	ar cq $(CCROOT)/lib/hc11/libc.a
	cp support8080/crt0.o $(CCROOT)/lib/8080/
	cp support8085/crt0.o $(CCROOT)/lib/8085/
	cp support8080/lib8080.a $(CCROOT)/lib/8080/lib8080.a
	cp support8085/lib8085.a $(CCROOT)/lib/8085/lib8085.a
	ar cq $(CCROOT)/lib/8080/libc.a
	cp supportz8/crt0.o $(CCROOT)/lib/z8/
	cp supportz8/libz8.a $(CCROOT)/lib/z8/libz8.a
	ar cq $(CCROOT)/lib/z8/libc.a
	cp supportee200/crt0.o $(CCROOT)/lib/ee200/
	cp supportee200/libee200.a $(CCROOT)/lib/ee200/libee200.a
	ar cq $(CCROOT)/lib/ee200/libc.a
	cp supportnova3/crt0.o $(CCROOT)/lib/nova/
	cp supportnova3/libnova.a $(CCROOT)/lib/nova/libnova.a
	ar cq $(CCROOT)/lib/nova/libc.a
	cp supportsuper8/crt0.o $(CCROOT)/lib/super8/
	cp supportsuper8/libsuper8.a $(CCROOT)/lib/super8/libsuper8.a
	ar cq $(CCROOT)/lib/super8/libc.a
	cp supportz80/crt0.o $(CCROOT)/lib/z80/
	cp supportz80/libz80.a $(CCROOT)/lib/z80/libz80.a
	ar cq $(CCROOT)/lib/z80/libc.a

#
#	Build the tools then install them
#
bootstrap: bootstuff bootinst

#
#	Build everything
#
install: bootstuff bootinst all libinst
