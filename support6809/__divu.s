;
;	D = TOS / D (unsigned)
;

	.export __divu
	.code

__divu:
	ldx 2,s			; get top of stack
	jsr div16x16		; D is now quotient
	exg d,x
pop2:
	ldx ,s
	leas 4,s
	jmp ,x

_remu:
	ldx 2,s			; get top of stack
	jsr div16x16		; D is now quotient
	bra pop2
	