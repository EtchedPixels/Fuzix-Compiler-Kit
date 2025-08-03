;
;	32bit compare used to build all the flag checks for
;	all the long comparisons. Idea taken from cc65 but we implement
;	it quite differently as we eventually want to shortcircuit as much
;	as possible though directly storing/working off tmp/tmp1
;

	.export __cceql
	.export __ccnel
	.export __ccgtl
	.export __ccgteql
	.export __ccgtul
	.export __ccgtequl
	.export __ccltl
	.export __cclteql
	.export __ccltul
	.export __ccltequl


__lcmp:
	jsr	__pop32
	; We are now comparing hireg:XA with tmp1:tmp
	sta	@tmp2
	lda	@hireg+1
	sec
	sbc	@tmp1+1
	bne	signs
	lda	@hireg
	cmp	@tmp1
	bne	differ
	txa
	cmp	@tmp+1
	bne	differ
	lda	@tmp2
	cmp	@tmp
differ:
	beq	done
	bcs	clear_n
	lda	#0xFF
done:	rts

clear_n:
	lda	#0x01
	rts

;
;	The top of the comparison is different as we need to get the
;	signed comparison checks righ ttoo
;
signs:
	bvc	done
	eor	#0xFF
	ora	#0x01
	rts

;
;	Condition checks
;
is_eq:
	beq	ret1
	lda	#0
	tax
	rts
is_ne:
	bne	ret1
	lda	#0
	tax
	rts
is_gt:
	beq	ret0
is_ge:
	bmi	ret0
ret1:
	lda	#1
	ldx	#0
	rts

is_le:
	beq	ret1
is_lt:
	bmi	ret1
	lda	#0
	tax
	rts

is_ugt:
	beq	ret0
is_uge:
	ldx	#0
	txa
	rol	a
	rts

is_ule:
	beq	ret1
is_ult:
	bcc	ret1
	lda	#0
	tax
	rts

__ccgtl:
	jsr	__lcmp
	jmp	is_gt

__ccgtul:
	jsr	__lcmp
	jmp	is_ugt

__ccgteql:
	jsr	__lcmp
	jmp	is_ge

__ccgtequl:
	jsr	__lcmp
	jmp	is_uge

__ccltl:
	jsr	__lcmp
	jmp	is_lt

__ccltul:
	jsr	__lcmp
	jmp	is_ult

__cclteql:
	jsr	__lcmp
	jmp	is_le

__ccltequl:
	jsr	__lcmp
	jmp	is_ule

__cceql:
	jsr	__lcmp
	jmp	is_eq

__ccnel:
	jsr	__lcmp
	jmp	is_ne
