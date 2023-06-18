all: emu85 testcrt0.o emuz80 testcrtz80.o

emu85: emu85.o intel_8085_emulator.o
	$(CC) emu85.o intel_8085_emulator.o -o emu85

emu85.o: emu85.c intel_8085_emulator.h

inte_8085_emulator.o: intel_8085_emulator.c intel_8085_emulator.h

emuz80: emuz80.o z80dis.o
	(cd libz80; make)
	$(CC) emuz80.o libz80/libz80.o z80dis.o -o emuz80

emuz80.o: emuz80.c libz80/libz80.o

z80dis.o: z80dis.h

testcrt0.o: testcrt0.s
	cc85 -c testcrt0.s

testcrtz80.o: testcrtz80.s
	ccz80 -c testcrtz80.s

clean:
	rm -f *.o tests/*.o *~ tests/*~ emu85 tests/*.map *.log emuz80
	(cd libz80; make clean)
