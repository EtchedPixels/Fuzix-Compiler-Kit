all: fcc cc0 \
     cc1.8080 cc1.6803 cc1.6809 cc1.z80 cc1.thread cc1.byte cc1.6502 \
     cc1.65c816 cc1.z8 cc1.1802 cc1.6800 \
     cc2 cc2.8080 cc2.6809 cc2.z80 cc2.65c816 cc2.6803 cc2.thread \
     cc2.6502 cc2.z8 cc2.super8 cc2.1802 cc2.6800 \
     copt support6502 support65c816 support6800 support6803 \
     support8080 support8085 supportz80 \
     supportz8 supportsuper8 test

bootstuff: cc cc0 \
     cc1.8080 cc1.6803 cc1.6809 cc1.z80 cc1.thread cc1.byte cc1.6502 \
     cc1.65c816 cc1.z8 cc1.super8 cc1.1802 cc1.6800 \
     cc2 cc2.8080 cc2.6809 cc2.z80 cc2.65c816 cc2.6803 cc2.thread \
     cc2.6502 cc2.z8 cc2.super8 cc2.1802 cc2.6800 \
     copt

.PHONY: support6502 support65c816 support6800 support6803 \
	support8080 support8085 supportsuper8 supportz8 supportz80 test

CCROOT ?=/opt/fcc/

OBJS0 = frontend.o

OBJS1 = body.o declaration.o enum.o error.o expression.o header.o idxdata.o \
	initializer.o label.o lex.o main.o primary.o stackframe.o storage.o \
	struct.o switch.o symbol.o tree.o type.o type_iterator.o

OBJS2 = backend.o backend-default.o
OBJS3 = backend.o backend-8080.o
OBJS4 = backend.o backend-6809.o
OBJS5 = backend.o backend-z80.o
OBJS6 = backend.o backend-65c816.o
OBJS7 = backend.o backend-6803.o
OBJS8 = backend.o backend-8070.o
OBJS9 = backend.o backend-threadcode.o
OBJS11 = backend.o backend-6502.o
OBJS12 = backend.o backend-65c816.o
OBJS13 = backend.o backend-z8.o
OBJS14 = backend.o backend-super8.o
OBJS15 = backend.o backend-1802.o
OBJS16 = backend.o backend-6800.o

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

fcc:	cc.o
	gcc -g3 cc.c -o fcc0

cc0:	$(OBJS0)
	gcc -g3 $(OBJS0) -o cc0

cc1.8080:$(OBJS1) target-8080.o
	gcc -g3 $(OBJS1) target-8080.o -o cc1.8080

cc1.z80:$(OBJS1) target-z80.o
	gcc -g3 $(OBJS1) target-z80.o -o cc1.z80

cc1.6803:$(OBJS1) target-6803.o
	gcc -g3 $(OBJS1) target-6803.o -o cc1.6803

cc1.6809:$(OBJS1) target-6809.o
	gcc -g3 $(OBJS1) target-6809.o -o cc1.6809

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

cc2:	$(OBJS2)
	gcc -g3 $(OBJS2) -o cc2

cc2.8080:	$(OBJS3)
	gcc -g3 $(OBJS3) -o cc2.8080

cc2.6809:	$(OBJS4)
	gcc -g3 $(OBJS4) -o cc2.6809

cc2.z80:	$(OBJS5)
	gcc -g3 $(OBJS5) -o cc2.z80

cc2.6803:	$(OBJS7)
	gcc -g3 $(OBJS7) -o cc2.6803

cc2.8070:	$(OBJS8)
	gcc -g3 $(OBJS8) -o cc2.8070

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

support6502:
	(cd support6502; make)

support65c816:
	(cd support65c816; make)

support6800:
	(cd support6800; make)

support6803:
	(cd support6803; make)

support8080:
	(cd support8080; make)

support8085:
	(cd support8085; make)

supportsuper8:
	(cd supportsuper8; make)

supportz8:
	(cd supportz8; make)

supportz80:
	(cd supportz80; make)

test:
	(cd test; make)

clean:
	rm -f cc cc85 ccz80 ccthread cc0 copt
	rm -f cc6502 cc65c816
	rm -f cc1.8080 cc1.z80 cc1.6803 cc1.thread
	rm -f cc1.6502 cc1.65c816 cc1.6809 cc1.byte
	rm -f cc2.8080 cc2.6809 cc2.z80 cc2.65c816 cc2.6803
	rm -f cc2.8070 cc2.thread cc2.byte cc2.6502
	rm -f cc1.super8 cc2.super8
	rm -f cc1.z8 cc2.z8
	rm -f *~ *.o
	(cd support6502; make clean)
	(cd support65c816; make clean)
	(cd support6800; make clean)
	(cd support6803; make clean)
	(cd support8080; make clean)
	(cd support8085; make clean)
	(cd supportz80; make clean)
	(cd supportz8; make clean)
	(cd supportsuper8; make clean)

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
	mkdir -p $(CCROOT)/lib/6800
	mkdir -p $(CCROOT)/lib/6800/include
	mkdir -p $(CCROOT)/lib/6803
	mkdir -p $(CCROOT)/lib/6803/include
	cp cc1.6800 $(CCROOT)/lib
	cp cc2.6800 $(CCROOT)/lib
	cp copt $(CCROOT)/lib
	cp rules.6800 $(CCROOT)/lib
	# 8080/8085
	mkdir -p $(CCROOT)/lib/8080
	mkdir -p $(CCROOT)/lib/8080/include
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
#	cp support6800/crt0.o $(CCROOT)/lib/6800/
#	cp support6800/lib6800.a $(CCROOT)/lib/6800/lib6800.a
	ar cq $(CCROOT)/lib/6800/libc.a
	cp support6803/crt0.o $(CCROOT)/lib/6800/
	cp support6803/lib6803.a $(CCROOT)/lib/6803/lib6803.a
	ar cq $(CCROOT)/lib/6803/libc.a
	cp support8085/crt0.o $(CCROOT)/lib/8080/
	cp support8080/lib8080.a $(CCROOT)/lib/8080/lib8080.a
	cp support8085/lib8085.a $(CCROOT)/lib/8080/lib8085.a
	ar cq $(CCROOT)/lib/8080/libc.a
	cp supportz8/crt0.o $(CCROOT)/lib/z8/
	cp supportz8/libz8.a $(CCROOT)/lib/z8/libz8.a
	ar cq $(CCROOT)/lib/z80/libc.a
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
