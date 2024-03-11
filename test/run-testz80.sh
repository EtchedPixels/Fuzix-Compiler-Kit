#!/bin/sh
for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	fcc -O -mz80 -c tests/$b.c
	ldz80 -b -C0 testcrtz80.o tests/$b.o -o tests/$b /opt/fcc/lib/z80/libz80.a -m tests/$b.map
	./emuz80 tests/$b tests/$b.map
	rm -f tests/$b tests/$b.o tests/$b.map
done
