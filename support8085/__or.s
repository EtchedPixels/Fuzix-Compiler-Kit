;
;	Compute HL = TOS | HL
;
		.export __or
		.export __oru

		.setcpu 8085
		.code
__or:
__oru:
		xchg			; working register into DE
		pop	h		; return address
		shld	__retaddr
		pop	h		; top of stack for maths
		mov	a,h
		ora	d
		mov	h,a
		mov	a,l
		ora	e
		mov	l,a
		jmp	__ret
