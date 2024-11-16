OBJ = cpp.o hash.o main.o token1.o token2.o

CFLAGS = -O2 -Wall -pedantic

all: cpp

.c.o:
	$(CC) $(CFLAGS) $(CCOPTS) -c $<

cpp: $(OBJ)

clean:
	rm -f *.o cpp *~
	
main.o: cc.h
cpp.o: cc.h
hash.o: cc.h
tree.o: cc.h

token1.o: token1.h
token2.o: token2.h
