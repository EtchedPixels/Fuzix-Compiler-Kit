	.setcpu 4
	.export __ccgteql
	.export __ccgtequl

	.code

;	TOS  < hireg:B
__ccgteql:
	; Long math we want to do hireg first
	stx (s+)
	lda (__hireg)
	ldx 4(s)		; high word
	; A = false - A
	sub x,a
	bz ccgteq2
	ble false
true:	ldb 1
	ldx (-s)
	rsr
false:
	clr b
	ldx (-s)
	rsr

__ccgtequl:
	stx (s+)
	lda (__hireg)
	ldx 4(s)
	sub x,a
	bl false
	bnl true
ccgteq2:
	lda 6(s)
	; B = A - B
	sab
	bl false
	bra true
