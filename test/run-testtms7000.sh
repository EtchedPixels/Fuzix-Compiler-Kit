#!/bin/sh

for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	fcc -X -mtms7000 -c tests/$b.c
	ld7000 -b -C0x200 -m tests/$b.map -o tests/$b testcrt0_tms7000.o tests/$b.o /opt/fcc/lib/tms7000/libtms7000.a
	./emu7k tests/$b tests/$b.map
done
