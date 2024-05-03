;
;	,X over Y:D
;
	.export __xdivequl
	.code

__xdivequl:
	staa	@tmp
	stab	@tmp+1
	ldaa	2,x
	ldab	3,x
	pshb
	psha
	ldaa	,x
	ldab	1,x
	pshb
	psha
	des
	des	; dummy
	ldaa	@tmp
	ldab 	@tmp+1
	pshb	; save old D at 2,s
	psha
	ldaa	@hireg
	ldab	@hireg+1
	pshb	; save hireg at 0,s
	psha
	stx	@tmp1
	tsx
	inx
	inx	; point to the constructed frame
	jsr	div32x32
	; Divison done now unpack the result
	ldaa	6,x
	ldab	7,x
	staa	@hireg
	stab	@hireg+1
	ldaa	8,x
	ldab	9,x
	staa	@tmp
	stab	@tmp+1
	ldx	@tmp1
	ldaa	@hireg
	ldab	@hireg+1
	staa	,x
	stab	1,x
	ldaa	@tmp
	ldab	@tmp+1
	staa	2,x
	stab	3,x
	jmp	__popret10
