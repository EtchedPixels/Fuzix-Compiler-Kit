;
;		True if TOS >= HL
;
		.export __ccgteq

		.setcpu 8085

		.code
;
__ccgtequ:
		xchg
		pop	h
		shld	__retaddr
		pop	h
		mov	a,l
		sub	e
		mov	a,h
		sbb	d
		lxi	h,0
		rc
		inr	l
		ret
