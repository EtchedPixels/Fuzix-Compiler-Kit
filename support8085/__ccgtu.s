;
;		True if TOS < HL
;
		.export __ccgtu

		.setcpu 8085

		.code
;
;	FIXME: flags as well as HL should be set up
;
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
		rc
		rz
		inr	l
		ret
