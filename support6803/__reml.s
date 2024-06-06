	.export __reml
	.code

	.setcpu 6803
__reml:
	std	@tmp
	ldaa	@hireg
	bita	#0x80
	beq	nosignfix
	ldd	@tmp
	jsr	__negatel
	std	@tmp
nosignfix:
	ldd	@tmp
	pshb
	psha
	ldd	@hireg
	pshb
	psha
	tsx
	;
	;	Sign check
	;
	ldaa	6,x
	staa	@tmp4
	bpl	nosignfix2
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
	ins
	ins
	ins
	ins	; arg cleanup
	;
	;	Get the result
	;
	ldd	@tmp2
	std	@hireg
	ldaa	@tmp4
	bpl	done
	ldd	@tmp3
	jsr	__negatel
	jmp	,x
done:
	ldd	@tmp3
	jmp	,x
