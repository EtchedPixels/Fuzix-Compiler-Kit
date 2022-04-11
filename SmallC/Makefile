
LIBDIR	=/usr/bin
LDFLAGS	=

DEFS	=

OBJS = initials.o data.o error.o expr.o function.o gen.o io.o \
       lex.o main.o primary.o stmt.o struct.o sym.o tree.o while.o

OBJ_8080 = code8080.o

CFLAGS = -Wall -pedantic

INC = data.h defs.h prototype.h

OBJS_ALL = $(OBJS) $(OBJ_8080)
all:    scc8080

scc8080: $(OBJS) $(OBJ_8080)
	$(CC) -g3 -o scc8080 $(OBJS) $(OBJ_8080)

$(OBJS_ALL) : $(INC)

clean:
	rm -f scc8080
	rm -f *~ *.o
	rm -f *.asm
	rm -f *.rel
	rm -f *.map
	rm -f *.lk
