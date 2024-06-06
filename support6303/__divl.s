	.export __divl
	.code

	.setcpu 6303
__divl:
	clr	@tmp4		; Sign tracking
	xgdx			; save low word in X
	ldaa	@hireg
	bita	#0x80
	beq	nosignfix
	xgdx			; low word back
	jsr	__negatel
	;	hireg:D now negated
	inc	@tmp4
	xgdx	; save again
nosignfix:
	pshx	; stack low word
	ldx	@hireg
	pshx	; stack high word

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
	tsx
	jsr	div32x32
	pulx
	pulx
	pulx	; return
	ldab	@tmp4
	rorb
	pula
	pulb
	std	@hireg
	bcc	nosignfix3
	pula
	pulb
	jsr	__negatel
	jmp	,x
nosignfix3:
	pula
	pulb
	jmp	,x
