	.export __divl
	.code

__divl:
	clr	@tmp4		; Sign tracking
	xgdy
	bita	#0x80
	beq	nosignfix
	xgdy
	jsr	__negatel
	inc	@tmp4
	xgdy
nosignfix:
	pshy
	pshb
	psha
	tsx
	;
	;	Now do other argument
	;
	ldaa	6,x
	bpl	nosignfix2
	inc	@tmp4
	ldd	8,x
	subd	#1
	std	8,x
	ldd	6,x
	sbcb	#0
	sbca	#0
	coma
	comb
	std	6,x
	com	8,x
	com	9,x
nosignfix2:
	jsr	div32x32
	pulx
	pulx
	pulx	; return
	ldab	@tmp4
	rorb
	bcc	nosignfix3
	puly
	pula
	pulb
	jsr	__negatel
	jmp	,x
nosignfix3:
	puly
	pula
	pulb
	jmp	,x
