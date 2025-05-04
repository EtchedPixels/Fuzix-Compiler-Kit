#!/bin/sh
for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	fcc -Os -m8080 -c tests/$b.c
	ld8080 -b -C0 testcrt0_8080.o tests/$b.o -o tests/$b /opt/fcc/lib/8080/lib8080.a -m tests/$b.map
	./emu85 tests/$b tests/$b.map
done
