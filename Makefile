all: cc85 ccz80 ccthread ccbyte cc6502 cc65c816 cc0 \
     cc1.8080 cc1.6803 cc1.6809 cc1.z80 cc1.thread cc1.byte cc1.6502 \
     cc1.65c816 \
     cc2 cc2.8080 cc2.6809 cc2.z80 cc2.65c816 cc2.6803 cc2.thread \
     cc2.byte cc2.6502 \
     copt support6502 support65c816 support8080 support8085 supportz80

.PHONY: support6502 support65c816 support8080 support8085 supportz80

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
OBJS10 = backend.o backend-bytecode.o
OBJS11 = backend.o backend-6502.o
OBJS12 = backend.o backend-65c816.o

CFLAGS = -Wall -pedantic -g3

INC0 = token.h
INC1 = body.h compiler.h declaration.h enum.h error.h expression.h header.h \
       idxdata.h initializer.h label.h lex.h primary.h stackframe.h storage.h \
       struct.h symbol.h target.h token.h tree.h type.h type_iterator.h
INC2 = backend.h symtab.h


$(OBJS0): $(INC0) symtab.h

$(OBJS1): $(INC1)

$(OBJS2): $(INC1) $(INC2)

$(OBJS3): $(INC1) $(INC2)

cc85:	cc85.o
	gcc -g3 cc85.o -o cc85

ccz80:	ccz80.o
	gcc -g3 ccz80.o -o ccz80

ccbyte: ccbyte.o
	gcc -g3 ccbyte.o -o ccbyte

ccthread: ccthread.o
	gcc -g3 ccthread.o -o ccthread

cc6502: cc6502.o
	gcc -g3 cc6502.o -o cc6502

cc65c816: cc65c816.o
	gcc -g3 cc65c816.o -o cc65c816

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

cc2.byte:	$(OBJS10)
	gcc -g3 $(OBJS10) -o cc2.byte

cc2.6502:	$(OBJS11)
	gcc -g3 $(OBJS11) -o cc2.6502

cc2.65c816:	$(OBJS12)
	gcc -g3 $(OBJS12) -o cc2.65c816

support6502:
	(cd support6502; make)

support65c816:
	(cd support65c816; make)

support8080:
	(cd support8080; make)

support8085:
	(cd support8085; make)

supportz80:
	(cd supportz80; make)

clean:
	rm -f cc cc85 ccz80 ccthread cc0 copt
	rm -f cc1.8080 cc1.z80 cc1.6803 cc1.thread
	rm -f  cc2.8080 cc2.6809 cc2.z80 cc2.65c816 cc2.6803 cc2.8070 cc2.thread
	rm -f *~ *.o
	(cd support6502; make clean)
	(cd support65c816; make clean)
	(cd support8080; make clean)
	(cd support8085; make clean)
	(cd supportz80; make clean)

doinstall:
	# 6502
	mkdir -p /opt/cc6502/bin
	mkdir -p /opt/cc6502/lib
	mkdir -p /opt/cc6502/include
	cp cc6502 /opt/cc6502/bin/cc6502
	cp cpp6502 /opt/cc6502/lib/cpp
	cp cc0 /opt/cc6502/lib
	cp cc1.6502 /opt/cc6502/lib
	cp cc2.6502 /opt/cc6502/lib
	cp copt /opt/cc6502/lib
	cp rules.6502 /opt/cc6502/lib
	cp support6502/crt0.o /opt/cc6502/lib
	cp support6502/lib6502.a /opt/cc6502/lib/lib6502.a
#	cp support6502/lib65c02.a /opt/cc6502/lib/lib65c02.a
	ar cq /opt/cc6502/lib/libc.a
	# 65c816
	mkdir -p /opt/cc65c816/bin
	mkdir -p /opt/cc65c816/lib
	mkdir -p /opt/cc65c816/include
	cp cc65c816 /opt/cc65c816/bin/cc65c816
	cp cpp65c816 /opt/cc65c816/lib/cpp
	cp cc0 /opt/cc65c816/lib
	cp cc1.65c816 /opt/cc65c816/lib
	cp cc2.65c816 /opt/cc65c816/lib
	cp copt /opt/cc65c816/lib
	cp rules.65c816 /opt/cc65c816/lib
	cp support65c816/crt0.o /opt/cc65c816/lib
	cp support65c816/lib65c816.a /opt/cc65c816/lib/lib65c816.a
	ar cq /opt/cc65c816/lib/libc.a
	# 8080/8085
	mkdir -p /opt/cc85/bin
	mkdir -p /opt/cc85/lib
	mkdir -p /opt/cc85/include
	cp cc85 /opt/cc85/bin/cc85
	cp cpp85 /opt/cc85/lib/cpp
	cp cc0 /opt/cc85/lib
	cp cc1.8080 /opt/cc85/lib
	cp cc2.8080 /opt/cc85/lib
	cp copt /opt/cc85/lib
	cp rules.8080 /opt/cc85/lib
	cp rules.8085 /opt/cc85/lib
	cp support8085/crt0.o /opt/cc85/lib
	cp support8080/lib8080.a /opt/cc85/lib/lib8080.a
	cp support8085/lib8085.a /opt/cc85/lib/lib8085.a
	ar cq /opt/cc85/lib/libc.a
	# Z80
	mkdir -p /opt/ccz80/bin
	mkdir -p /opt/ccz80/lib
	mkdir -p /opt/ccz80/include
	cp ccz80 /opt/ccz80/bin/ccz80
	cp cppz80 /opt/ccz80/lib/cpp
	cp cc0 /opt/ccz80/lib
	cp cc1.z80 /opt/ccz80/lib
	cp cc2.z80 /opt/ccz80/lib
	cp copt /opt/ccz80/lib
	cp rules.z80 /opt/ccz80/lib
	cp supportz80/crt0.o /opt/ccz80/lib
	cp supportz80/libz80.a /opt/ccz80/lib/libz80.a
	ar cq /opt/ccz80/lib/libc.a
	# Threadcode
	mkdir -p /opt/ccthread/bin
	mkdir -p /opt/ccthread/lib
	mkdir -p /opt/ccthread/include
	cp ccthread /opt/ccthread/bin/ccthread
	cp cppthread /opt/ccthread/lib/cpp
	cp cc0 /opt/ccthread/lib
	cp cc1.thread /opt/ccthread/lib
	cp cc2.thread /opt/ccthread/lib
	cp copt /opt/ccthread/lib
	cp rules.thread /opt/ccthread/lib
	# Bytecode
	mkdir -p /opt/ccbyte/bin
	mkdir -p /opt/ccbyte/lib
	mkdir -p /opt/ccbyte/include
	cp ccbyte /opt/ccbyte/bin/ccbyte
	cp cppbyte /opt/ccbyte/lib/cpp
	cp cc0 /opt/ccbyte/lib
	cp cc1.byte /opt/ccbyte/lib
	cp cc2.byte /opt/ccbyte/lib
	cp copt /opt/ccbyte/lib
	cp rules.byte /opt/ccbyte/lib
#	cp supportthread/crt0.o /opt/ccthread/lib
#	cp supportthread/libthread.a /opt/ccthread/lib/libthread.a
#	ar cq /opt/ccthread/lib/libc.a

# assumes a suitable cpp, as, libs and includes are present
# doinstall does a bootstrap install
install: all doinstall
