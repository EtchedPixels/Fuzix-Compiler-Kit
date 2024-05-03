#!/bin/sh
for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	fcc -m68hc11 -c tests/$b.c
	ld6800 -b -C32768 testcrt0_6803.o tests/$b.o -o tests/$b /opt/fcc/lib/hc11/libhc11.a -m tests/$b.map
	./emu6800 6811 tests/$b tests/$b.map
done
