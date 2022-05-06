all: cc0 cc1 cc2

OBJS0 = frontend.o

OBJS1 = body.o declaration.o error.o expression.o header.o idxdata.o \
	initializer.o label.o lex.o main.o primary.o stackframe.o storage.o \
	struct.o symbol.o target-8080.o tree.o type.o

OBJS2 = backend.o backend-8080.o

CFLAGS = -Wall -pedantic -g3

INC0 = token.h
INC1 = body.h compiler.h declaration.h error.h expression.h header.h \
       idxdata.h initializer.h label.h lex.h primary.h stackframe.h storage.h \
       struct.h symbol.h target.h token.h tree.h type.h
INC2 = backend.h


$(OBJS0): $(INC0)

$(OBJS1): $(INC1)

$(OBJS2): $(INC1) $(INC2)

cc0:	$(OBJS0)
	gcc -g3 $(OBJS0) -o cc0

cc1:	$(OBJS1)
	gcc -g3 $(OBJS1) -o cc1

cc2:	$(OBJS2)
	gcc -g3 $(OBJS2) -o cc2

clean:
	rm -f cc0 cc1 cc2
	rm -f *~ *.o
