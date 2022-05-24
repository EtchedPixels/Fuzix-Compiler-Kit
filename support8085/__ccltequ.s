;
;		True if TOS < HL
;
		.export __ccltequ

		.setcpu 8085

		.code

__ccltequ:
		xchg
		pop	h
		xthl
		mov	a,l
		sub	e
		mov	a,h
		sbb	d
		jc	__true
		jmp	__false
