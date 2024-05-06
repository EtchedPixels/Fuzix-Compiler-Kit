	.setcpu 4
	.export __ccltl
	.export __ccltul

	.code

;	TOS  < hireg:B
__ccltl:
	; Long math we want to do hireg first
	stx (s+)
	lda (__hireg)
	ldx 4(s)		; high word
	sub x,a
	bz cclt2
	ble true
false:	clr b
	ldx (-s)
	rsr
true:
	ldb 1
	ldx (-s)
	rsr

__ccltul:
	stx (s+)
	lda (__hireg)
	ldx 4(s)
	sub x,a
	bl true
	bnl false
cclt2:
	lda 6(s)
	; B = A - B
	sab
	bl true
	bra false
