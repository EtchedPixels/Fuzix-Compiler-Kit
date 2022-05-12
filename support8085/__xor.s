;
;	Compute HL = TOS ^ HL
;
		.export __xor
		.export __xoru

		.setcpu 8085
		.code
__xor:
__xoru:
		xchg			; working register into DE
		pop	h		; return address
		shld	__retaddr
		pop	h		; top of stack for maths
		mov	a,h
		xra	d
		mov	h,a
		mov	a,l
		xra	e
		mov	l,a
		jmp	__ret
