	.setcpu 4
	.export __cclteql
	.export __ccltequl

	.code

;	TOS  < hireg:B
__cclteql:
	; Long math we want to do hireg first
	stx (s+)
	lda (__hireg)
	ldx 4(s)		; high word
	sub a,x
	bz cclteq2
	ble false
true:	ldb 1
	ldx (-s)
	rsr
false:
	clr b
	ldx (-s)
	rsr

__ccltequl:
	stx (s+)
	lda (__hireg)
	ldx 4(s)
	sub a,x
	bl false
	bnl true
cclteq2:
	lda 6(s)
	sub b,a
	bl false
	bra true
