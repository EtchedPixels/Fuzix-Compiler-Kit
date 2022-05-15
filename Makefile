all: cc cc0 cc1 cc2 cc2.8080 cc2.6809 cc2.z80 cc2.65c816 support8085

.PHONY: support8085

OBJS0 = frontend.o

OBJS1 = body.o declaration.o enum.o error.o expression.o header.o idxdata.o \
	initializer.o label.o lex.o main.o primary.o stackframe.o storage.o \
	struct.o switch.o symbol.o target-8080.o tree.o type.o type_iterator.o

OBJS2 = backend.o backend-default.o
OBJS3 = backend.o backend-8080.o
OBJS4 = backend.o backend-6809.o
OBJS5 = backend.o backend-z80.o
OBJS6 = backend.o backend-65c816.o

CFLAGS = -Wall -pedantic -g3

INC0 = token.h
INC1 = body.h compiler.h declaration.h enum.h error.h expression.h header.h \
       idxdata.h initializer.h label.h lex.h primary.h stackframe.h storage.h \
       struct.h symbol.h target.h token.h tree.h type.h type_iterator.h
INC2 = backend.h


$(OBJS0): $(INC0)

$(OBJS1): $(INC1)

$(OBJS2): $(INC1) $(INC2)

$(OBJS3): $(INC1) $(INC2)

cc:	cc.o
	gcc -g3 cc.o -o cc

cc0:	$(OBJS0)
	gcc -g3 $(OBJS0) -o cc0

cc1:	$(OBJS1)
	gcc -g3 $(OBJS1) -o cc1

cc2:	$(OBJS2)
	gcc -g3 $(OBJS2) -o cc2

cc2.8080:	$(OBJS3)
	gcc -g3 $(OBJS3) -o cc2.8080

cc2.6809:	$(OBJS4)
	gcc -g3 $(OBJS4) -o cc2.6809

cc2.z80:	$(OBJS5)
	gcc -g3 $(OBJS5) -o cc2.z80

cc2.65c816:	$(OBJS6)
	gcc -g3 $(OBJS6) -o cc2.65c816

support8085:
	(cd support8085; make)

clean:
	rm -f cc cc0 cc1 cc2 cc2.8080 cc2.6809
	rm -f *~ *.o
	(cd support8085; make clean)

# Hack for now
# assumes a suitable cpp, as, libs and includes are present
install: all
	mkdir -p /opt/cc85/bin
	mkdir -p /opt/cc85/lib
	mkdir -p /opt/cc85/include
	cp cc /opt/cc85/bin/cc85
	cp cc[01] /opt/cc85/lib
	cp cc2.8080 /opt/cc85/lib
	cp support8085/crt0.o /opt/cc85/lib
	cp support8085/lib8085.a /opt/cc85/lib/lib8085.a
	ar cq /opt/cc85/lib/libc.a
