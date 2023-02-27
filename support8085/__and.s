;
;	Compute HL = TOS & HL
;
		.export __band

		.setcpu 8080
		.code
__band:
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
