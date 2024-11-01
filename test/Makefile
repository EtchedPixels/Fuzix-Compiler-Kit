all: emu6502 emu65c816 emu6800 emu85 emuz8 emuz80 byte1802 emu6809 ee200 nova \
     testcrt0.o testcrtz80.o testcrt0_6502.o testcrt0_65c816.o \
     testcrt0_6303.o testcrt0_6803.o testcrt0_6809.o testcrt0_z8.o \
     testcrt0_byte1802.o testcrt0_ee200.o testcrt0_nova.o testcrt0_nova3.o \
     testcrt0_6800.o

emu85: emu85.o intel_8085_emulator.o
	$(CC) emu85.o intel_8085_emulator.o -o emu85

emu85.o: emu85.c intel_8085_emulator.h

emu6800.o: emu6800.c 6800.h

intel_8085_emulator.o: intel_8085_emulator.c intel_8085_emulator.h

CFLAGS += -O2 -Wall -pedantic

byte1802: byte1802.o ../support1802/1802ops.h
	$(CC) byte1802.o -o byte1802

emuz8: emuz8.o z8.o
	$(CC) emuz8.o z8.o -o emuz8

emuz80.o: emuz80.c
	(cd libz80; make)
	$(CC) $(CFLAGS) -c emuz80.c

emuz80: emuz80.o z80dis.o
	(cd libz80; make)
	$(CC) emuz80.o libz80/libz80.o z80dis.o -o emuz80

emu6502: emu6502.o 6502.o 6502dis.o
	$(CC) emu6502.o 6502.o 6502dis.o -o emu6502

emu65c816: emu65c816.o 
	(cd lib65c816; make)
	$(CC) emu65c816.o lib65c816/src/lib65816.a -o emu65c816

emu65c816.o: emu65c816.c
	(cd lib65c816; make)
	$(CC) $(CFLAGS) -c -Ilib65c816 emu65c816.c

6800.o: 6800.c 6800.h
	$(CC) $(CFLAGS) -DWITH_HC11 -c 6800.c

emu6800: 6800.o emu6800.o
	$(CC) $(CFLAGS) -o emu6800 emu6800.o 6800.o

e6809.o: e6809.c e6809.h
	$(CC) $(CFLAGS) -c e6809.c

d6809.o: d6809.c d6809.h e6809.h
	$(CC) $(CFLAGS) -c d6809.c

emu6809.o: emu6809.c d6809.h e6809.h
	$(CC) $(CFLAGS) -c emu6809.c

emu6809: emu6809.o e6809.o d6809.o
	$(CC) $(CFLAGS) -o emu6809 emu6809.o e6809.o d6809.o

ee200: ee200.o ee200_dis.o
	$(CC) $(CFLAGS) -o ee200 ee200.o ee200_dis.o

nova: nova.o
	$(CC) $(CFLAGS) -o nova nova.o

z80dis.o: z80dis.h

testcrt0.o: testcrt0.s
	fcc -m8085 -c testcrt0.s

testcrt0_byte1802.o: testcrt0_byte1802.S ../support1802/1802ops.h
	fcc -m1802 -c testcrt0_byte1802.S

testcrtz80.o: testcrtz80.s
	fcc -mz80 -c testcrtz80.s

testcrt0_6502.o: testcrt0_6502.s
	fcc -m6502 -c testcrt0_6502.s

testcrt0_65c816.o: testcrt0_65c816.s
	fcc -m65c816 -c testcrt0_65c816.s

testcrt0_6303.o: testcrt0_6303.s
	fcc -m6303 -c testcrt0_6303.s

testcrt0_6800.o: testcrt0_6800.s
	fcc -m6800 -c testcrt0_6800.s

testcrt0_6803.o: testcrt0_6803.s
	fcc -m6803 -c testcrt0_6803.s

testcrt0_6809.o: testcrt0_6809.s
	fcc -m6809 -c testcrt0_6809.s

testcrt0_ee200.o: testcrt0_ee200.s
	fcc -mee200 -c testcrt0_ee200.s

testcrt0_nova.o: testcrt0_nova3.s
	fcc -mnova -c testcrt0_nova.s

testcrt0_nova3.o: testcrt0_nova3.s
	fcc -mnova3 -c testcrt0_nova3.s

testcrt0_z8.o: testcrt0_z8.s
	fcc -mz8 -c testcrt0_z8.s

clean:
	rm -f *.o tests/*.o *~ tests/*~ emu85 tests/*.map *.log emuz80
	rm -f emu6502 byte1802 emu65c816 emuz8 emu6809 ee200 nova
	rm -f wtests/*.o
	(cd libz80; make clean)
	(cd lib65c816; make clean)

