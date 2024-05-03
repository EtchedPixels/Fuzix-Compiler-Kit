	.export __divl
	.code

__divl:
	clr	@tmp4		; Sign tracking
	staa	@tmp		; save low word in tmp
	stab	@tmp+1
	ldaa	@hireg
	bita	#0x80
	beq	nosignfix
	ldaa	@tmp		; low word back
	ldab	@tmp+1
	jsr	__negatel
	;	hireg:D now negated
	inc	@tmp4
	staa	@tmp	; save again
	stab	@tmp+1
nosignfix:
	ldaa	@hireg
	ldab	@hireg+1
	pshb
	psha
	ldaa	@tmp	; low word back
	ldab	@tmp+1
	pshb	; stack it
	psha
	tsx
	;
	;	Now do other argument
	;
	ldaa	6,x
	bpl	nosignfix2
	inc	@tmp4
	ldaa	8,x
	ldab	9,x
	subb	#1
	sbca	#0
	staa	8,x
	stab	9,x
	ldaa	6,x
	ldab	7,x
	sbcb	#0
	sbca	#0
	coma
	comb
	staa	6,x
	stab	7,x
	com	8,x
	com	9,x
nosignfix2:
	jsr	div32x32
	ins
	ins
	ins
	ins
	pula
	pulb
	tsx	; no PULX on 6800
	ldx ,x	; return
	ins
	ins
	ldab	@tmp4
	rorb
	ins
	ins
	bcc	nosignfix3
	pula
	pulb
	jsr	__negatel
	jmp	,x
nosignfix3:
	pula
	pulb
	jmp	,x
