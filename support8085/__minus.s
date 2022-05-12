;
;	Compute TOS - HL -> HL
;
		.export __minus
		.export __minusu

		.setcpu 8085
		.code
__minus:
__minusu:
		xchg			; working register into DE
		pop	h		; return address
		shld	__retaddr
		pop	h		; top of stack for maths
		push	b		; save working BC
		mov	b,d
		mov	c,e
		dsub
		pop	b		; get B back
		jmp	__ret
