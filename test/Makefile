all: emu6502 emu65c816 emu85 emuz8 emuz80 byte1802 \
     testcrt0.o testcrtz80.o testcrt0_6502.o testcrt0_65c816.o \
     testcrt0_z8.o testcrt0_byte1802.o

emu85: emu85.o intel_8085_emulator.o
	$(CC) emu85.o intel_8085_emulator.o -o emu85

emu85.o: emu85.c intel_8085_emulator.h

inte_8085_emulator.o: intel_8085_emulator.c intel_8085_emulator.h

CFLAGS += -O2 -Wall -pedantic

byte1802: byte1802.o ../support1802/1802ops.h
	$(CC) byte1802.o -o byte1802

emuz8: emuz8.o z8.o
	$(CC) emuz8.o z8.o -o emuz8

emuz80: emuz80.o z80dis.o
	(cd libz80; make)
	$(CC) emuz80.o libz80/libz80.o z80dis.o -o emuz80

emuz80.o: emuz80.c libz80/libz80.o

emu6502: emu6502.o 6502.o 6502dis.o
	$(CC) emu6502.o 6502.o 6502dis.o -o emu6502

emu65c816: emu65c816.o
	(cd lib65c816; make)
	$(CC) emu65c816.o lib65c816/src/lib65816.a -o emu65c816

emu65c816.o: emu65c816.c
	$(CC) $(CFLAGS) -c -Ilib65c816 emu65c816.c

z80dis.o: z80dis.h

testcrt0.o: testcrt0.s
	cc85 -c testcrt0.s

testcrt0_byte1802.o: testcrt0_byte1802.S ../support1802/1802ops.h
	fcc -m1802 -c testcrt0_byte1802.S

testcrtz80.o: testcrtz80.s
	ccz80 -c testcrtz80.s

testcrt0_6502.o: testcrt0_6502.s
	fcc -m6502 -c testcrt0_6502.s

testcrt0_65c816.o: testcrt0_65c816.s
	fcc -m65c816 -c testcrt0_65c816.s

testcrt0_z8.o: testcrt0_z8.s
	fcc -mz8 -c testcrt0_z8.s


clean:
	rm -f *.o tests/*.o *~ tests/*~ emu85 tests/*.map *.log emuz80
	rm -f emu6502
	(cd libz80; make clean)
	(cd lib65c816; make clean)

