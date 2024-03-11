#!/bin/sh
for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	fcc -m6803 -c tests/$b.c
	ld6800 -b -C512 testcrt0_6803.o tests/$b.o -o tests/$b /opt/fcc/lib/6803/lib6803.a -m tests/$b.map
	./emu6800 tests/$b tests/$b.map
done
