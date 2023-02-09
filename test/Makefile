all: emu85 testcrt0.o

emu85: emu85.o intel_8085_emulator.o
	$(CC) emu85.o intel_8085_emulator.o -o emu85

emu85.o: emu85.c intel_8085_emulator.h

inte_8085_emulator.o: intel_8085_emulator.c intel_8085_emulator.h

testcrt0.o: testcrt0.s
	cc85 -c testcrt0.s

clean:
	rm -f *.o tests/*.o *~ tests/*~ emu85 tests/*.map *.log
