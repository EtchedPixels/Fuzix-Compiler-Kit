#!/bin/sh
for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	fcc -mz8 -c tests/$b.c
	ldz8 -b -C0 -Z48 testcrt0_z8.o tests/$b.o -o tests/$b /opt/fcc/lib/z8/libz8.a -m tests/$b.map
	./emuz8 tests/$b tests/$b.map
done
