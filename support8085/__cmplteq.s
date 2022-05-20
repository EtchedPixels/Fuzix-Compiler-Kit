;
;		True if TOS <= HL
;
		.export __cmplteq

		.setcpu 8080

		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
;
__cmplteq:
		mov	a,h
		xra	d
		jp	sign_same
		xra	d		; A is now H
		jm	__rfalse
		jmp	__rtrue
sign_same:
		mov	a,e
		sub	l
		mov	a,d
		sbb	h
		lxi	h,2
		jz	__rtrue
		jc	__rtrue
		jmp	__rfalse
