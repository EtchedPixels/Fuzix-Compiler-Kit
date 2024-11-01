#!/bin/sh

for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	fcc -X -m6800 -c tests/$b.c
	ld6800 -b -C256 -Z0  -m tests/$b.map -o tests/$b testcrt0_6800.o tests/$b.o /opt/fcc/lib/6800/lib6800.a  /opt/fcc/lib/6800/libc.a
	./emu6800 6800 tests/$b tests/$b.map
	echo $?
done
