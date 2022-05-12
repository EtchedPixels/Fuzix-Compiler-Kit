;
;		True if TOS < HL
;
		.export __ccltequ

		.setcpu 8085

		.code
;
;	FIXME: flags as well as HL should be set up
;
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
		rc
		rz
		dcr	l
		ret
