#!/bin/sh

for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	fcc -X -m8070 -c tests/$b.c
	ld8070 -b -C1 -m tests/$b.map -o tests/$b testcrt0_8070.o tests/$b.o /opt/fcc/lib/8070/lib8070.a
	./emu807x tests/$b tests/$b.map
done
