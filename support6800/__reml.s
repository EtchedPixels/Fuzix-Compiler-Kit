	.export __reml
	.code

__reml:
	staa	@tmp
	stab	@tmp+1
	ldaa	@hireg
	bita	#0x80
	beq	nosignfix
	ldaa	@tmp
	ldab	@tmp+1
	jsr	__negatel
	staa	@tmp
	stab	@tmp+1
nosignfix:
	ldaa	@hireg
	ldab	@hireg+1
	pshb
	psha
	ldaa	@tmp
	ldab	@tmp+1
	pshb
	psha
	tsx
	;
	;	Sign check
	;
	ldaa	6,x
	staa	@tmp4
	bpl	nosignfix2
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
	;
	;	Get the result
	;
	ldaa	@tmp2
	ldab	@tmp2+1
	staa	@hireg
	stab	@hireg+1
	ldaa	@tmp4
	bpl	done
	ldaa	@tmp3
	ldab	@tmp3+1
	jsr	__negatel
	jmp	__pop4
done:
	ldaa	@tmp3
	ldab	@tmp3+1
	jmp	__pop4