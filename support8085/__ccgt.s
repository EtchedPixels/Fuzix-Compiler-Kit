;
;		True if TOS > HL
;
		.export __ccgt

		.setcpu 8085

		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
;
__ccgt:
		xchg
		pop	h
		shld	__retaddr
		pop	h
		mov	a,h
		xra	d
		jp	sign_same
		xra	d		; A is now H
		lxi	h,1
		jp	ret1
		jz	ret1
ret1:		inr	l
ret0:		dcr	l		; flags as well
		jmp	__ret
sign_same:
		mov	a,e
		sub	l
		mov	a,d
		sbb	h
		lxi	h,1
		jz	ret1
		jc	ret1
		jmp	ret0
