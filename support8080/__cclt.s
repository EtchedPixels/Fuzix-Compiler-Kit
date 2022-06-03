;
;		True if TOS < HL
;
		.export __cclt

		.setcpu 8080

		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
;
__cclt:
		xchg
		pop	h
		xthl
		mov	a,h
		xra	d
		jp	sign_same
		xra	d		; A is now H
		jm	__false
		jmp	__true
sign_same:
		; TOS is now in HL, old HL in DE, test  HL < DE
		mov	a,l
		sub	e
		mov	a,h
		sbb	d
		jc	__true
		jmp	__false
