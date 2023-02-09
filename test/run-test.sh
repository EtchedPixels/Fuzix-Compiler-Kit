#!/bin/sh
for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	cc85 -c tests/$b.c
	ld85 -b -C0 testcrt0.o tests/$b.o -o tests/$b
	./emu85 tests/$b
done

	