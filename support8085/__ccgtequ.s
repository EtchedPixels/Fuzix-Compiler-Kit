;
;		True if TOS >= HL
;
		.export __ccgtequ

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
; FIXME: flags
		lxi	h,0
		rc
		inr	l
		ret
