;
;	,X over hireg:D
;
	.export __xremequl
	.code

__xremequl:
	staa	@tmp
	stab	@tmp+1
	ldaa	2,x
	ldab	3,x
	pshb
	psha
	ldaa	0,x
	ldab	1,x
	pshb
	psha
	des
	des	; dummy
	ldaa	@tmp
	ldab	@tmp+1
	pshb	; Save D at 2,S in frame
	psha
	ldaa	@hireg
	ldab	@hireg+1
	pshb
	psha	; save hireg at 0,S in the frame
	stx	@tmp1
	tsx
;	inx
;	inx	; X points at the frame
	jsr	div32x32
	ldaa	@tmp2
	ldab	@tmp2+1
	staa	@hireg
	stab	@hireg+1
	ldx	@tmp1
	ldaa	@tmp2
	ldab	@tmp2+1
	staa	,x
	stab	1,x
	ldaa	@tmp3
	ldab	@tmp3+1
	staa	2,x
	stab	3,x
	jmp	__popret10

