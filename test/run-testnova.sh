#!/bin/sh
for i in tests/*.c
do
	b=$(basename $i .c)
	echo  $b":"
	fcc -O2 -mnova -c tests/$b.c
	ldnova -b -C512 -Z80 testcrt0_nova.o tests/$b.o -o tests/$b /opt/fcc/lib/nova/libnova.a -m tests/$b.map
	./nova tests/$b tests/$b.map
done
