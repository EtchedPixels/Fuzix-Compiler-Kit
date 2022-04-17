all: cc0 cc1

OBJS0 = frontend.o

OBJS1 = body.o declaration.o error.o expression.o header.o initializer.o \
	lex.o main.o primary.o stackframe.o storage.o symbol.o \
        tree.o type.o

CFLAGS = -Wall -pedantic -g3

INC0 = token.h
INC1 = body.h compiler.h declaration.h error.h expression.h header.h \
       initializer.h lex.h primary.h stackframe.h storage.h symbol.h \
       target.h token.h tree.h type.h


$(OBJS0): $(INC0)

$(OBJS1): $(INC1)

cc0:	$(OBJS0)
	gcc -g3 $(OBJS0) -o cc0

cc1:	$(OBJS1)
	gcc -g3 $(OBJS1) -o cc1

clean:
	rm -f cc0 cc1
	rm -f *~ *.o
