;
;	Compute HL = TOS & HL
;
		.export __xorc
		.export __xoruc

		.setcpu 8085
		.code
__xorc:
__xoruc:
		mov	a,l		; working register into A
		pop	h		; return address
		shld	__retaddr
		xra	e
		mov	l,a
		jmp	__ret
