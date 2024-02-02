#!/bin/sh
for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	ccz80 -O -mz80 -c tests/$b.c
	ldz80 -b -C0 testcrtz80.o tests/$b.o -o tests/$b /opt/ccz80/lib/libz80.a -m tests/$b.map
	./emuz80 tests/$b tests/$b.map
done
