;
;	D = top of stack * D
;
	.export __mul

	.code

__mul:
	pshb
	psha
	tsx
	; ,x and 1,x are D
	; 3,x and 4,x are the top of stack value
	ldab #16
	stab @tmp		; iteration count

	clra			; AB now becomes our 16bit
	clrb			; work register

	; Rotate through the number
nextbit:
	lsr ,x
	ror 1,x
	bcc noadd
	addb 5,x
	adca 4,x
noadd:	lsl  5,x
	rol  4,x
	dec @tmp
	bne nextbit
	; For a 16x16 to 32bit just store 3-4,x into sreg
	ins
	ins
	jmp __pop2

