#!/bin/sh
for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	fcc -m6303 -c tests/$b.c
	ld6800 -b -C256 -Z64 testcrt0_6303.o tests/$b.o -o tests/$b /opt/fcc/lib/6303/lib6303.a -m tests/$b.map
	./emu6800 6303 tests/$b tests/$b.map
done
