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
	; 4,x and 5,x are the top of stack value
	ldab #16
	stab @tmp		; iteration count

	clra			; AB now becomes our 16bit
	clrb			; work register

	; Rotate through the number
nextbit:
	aslb
	rola
	rol	5,x
	rol	4,x
	bcc noadd
	addb 1,x
	adca 0,x
noadd:
	dec @tmp
	bne nextbit
	; For a 16x16 to 32bit just store 4-5,x into sreg
	ins
	ins
	jmp __pop2

