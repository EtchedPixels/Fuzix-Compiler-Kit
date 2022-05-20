;
;		True if TOS < HL
;
		.export __ccgtu

		.setcpu 8085

		.code

__ccgtu:
		xchg
		pop	h
		shld	__retaddr
		pop	h
		mov	a,l
		sub	e
		mov	a,h
		sbb	d
		lxi	h,0
		; if C or Z then true
		jnz	retf
		inr	l	; return 1 set NZ
		ret
retf:		xra	a
		ret
