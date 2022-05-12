;
;	Compute HL = TOS & HL
;
		.export __and
		.export __andu

		.setcpu 8085
		.code
__and:
__andu:
		xchg			; working register into DE
		pop	h		; return address
		shld	__retaddr
		pop	h		; top of stack for maths
		mov	a,h
		ana	d
		mov	h,a
		mov	a,l
		ana	e
		mov	l,a
		jmp	__ret
