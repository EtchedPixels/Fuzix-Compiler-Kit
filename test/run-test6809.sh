#!/bin/sh
for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	fcc -m6809 -c tests/$b.c
	ld6809 -b -C512 testcrt0_6809.o tests/$b.o -o tests/$b /opt/fcc/lib/6809/lib6809.a -m tests/$b.map
	./emu6809 tests/$b tests/$b.map
done
