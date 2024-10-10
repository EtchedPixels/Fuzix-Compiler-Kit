	.export __divl
	.code

__divl:
	clr	@tmp4		; Sign tracking
	tst		@hireg
	bpl		nosignfix
	jsr	__negatel
	;	hireg:D now negated
	inc	@tmp4
nosignfix:
	pshb
	psha
	ldaa	@hireg
	ldab	@hireg+1
	pshb
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
	ldaa 6,x
	ldab 7,x
	staa @hireg
	stab @hireg+1
	ldaa 8,x
	ldab 9,x
	ldx 4,x
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ror		@tmp4
	bcc	nosignfix3
	jsr	__negatel
nosignfix3:
	jmp	,x
