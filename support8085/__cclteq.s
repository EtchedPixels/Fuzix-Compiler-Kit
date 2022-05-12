;
;		True if TOS <= HL
;
		.export __cclteq

		.setcpu 8085

		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
;
__cclteq:
		xchg
		pop	h
		shld	__retaddr
		pop	h
		mov	a,h
		xra	d
		jp	sign_same
		xra	d		; A is now H
		lxi	h,2
		jp	ret1
		jz	ret1
ret0:		dcr	l
ret1:		dcr	l		; flags as well
		jmp	__ret
sign_same:
		mov	a,e
		sub	l
		mov	a,d
		sbb	h
		lxi	h,2
		jz	ret0
		jc	ret0
		jmp	ret1
