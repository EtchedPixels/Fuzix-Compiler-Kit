	.setcpu 4
	.export __ccgtl
	.export __ccgtul

	.code

;	TOS  < hireg:B
__ccgtl:
	; Long math we want to do hireg first
	stx (s+)
	lda (__hireg)
	ldx 4(s)		; high word
	sub a,x
	bz ccgt2
	ble true
false:	clr b
	ldx (-s)
	rsr
true:
	ldb 1
	ldx (-s)
	rsr

__ccgtul:
	stx (s+)
	lda (__hireg)
	ldx 4(s)
	sub a,x
	bl true
	bnl false
ccgt2:
	lda 6(s)
	sub b,a
	bl true
	bra false
