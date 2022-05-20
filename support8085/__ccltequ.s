;
;		True if TOS < HL
;
		.export __ccltequ

		.setcpu 8085

		.code

__ccltequ:
		xchg
		pop	h
		shld	__retaddr
		pop	h
		mov	a,l
		sub	e
		mov	a,h
		sbb	d
		lxi	h,0
		; if C or Z then false
		jnz	rett
		xra	a
		ret
rett:		inr	l
		ret
