OBJ = cpp.o hash.o main.o token1.o token2.o

CFLAGS = -O2 -Wall -pedantic

all: cpp

.c.o:
	$(CC) $(CFLAGS) $(CCOPTS) -c $<

# Older Linux platform users may need libbsd-deevel / libbsd-dev
# and to uncomment -lbsd if they do not have glibc 2.38 or higher
cpp: $(OBJ)
        $(CC) $(CFLAGS) $(CCOPTS) $(OBJ) -o cpp #-lbsd

clean:
	rm -f *.o cpp *~
	
main.o: cc.h
cpp.o: cc.h
hash.o: cc.h
tree.o: cc.h

token1.o: token1.h
token2.o: token2.h
