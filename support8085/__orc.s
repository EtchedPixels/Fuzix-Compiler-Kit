;
;	Compute HL = TOS & HL
;
		.export __orc
		.export __oruc

		.setcpu 8085
		.code
__orc:
__oruc:
		mov	a,l		; working register into A
		pop	h		; return address
		shld	__retaddr
		ora	e
		mov	l,a
		jmp	__ret
