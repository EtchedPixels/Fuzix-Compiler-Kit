#!/bin/sh
for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	fcc -m6502 -c tests/$b.c
	ld6502 -b -C512 testcrt0.o tests/$b.o -o tests/$b /opt/fcc/lib/6502/lib6502.a -m tests/$b.map
	./emu6502 tests/$b tests/$b.map
done
