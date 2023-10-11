	.65c816
	.a16
	.i16

	.export __cceql
	.export __ccnel
	.export __ccgtl
	.export __ccltl
	.export __cclteql
	.export __ccgteql
	.export __ccequl
	.export __ccneul
	.export __ccgtul
	.export __ccltul
	.export __ccltequl
	.export __ccgtequl

	.export __cceqlz
	.export __ccnelz
	.export __ccgtlz
	.export __ccltlz
	.export __cclteqlz
	.export __ccgteqlz
	.export __ccequlz
	.export __ccneulz
	.export __ccgtulz
	.export __ccltulz
	.export __ccltequlz
	.export __ccgtequlz

;
;	Long compare
;
longcmp:
	; Compare hireg:A with 0-3,y
	tax
	lda @hireg
	cmp 2,y
	beq chklo
	rts
chklo:
	txa
	cmp 0,y
	rts


__cceqlz:
__ccequlz:
	stz @hireg
__cceql:
__ccequl:
	jsr longcmp
	beq true
false:
	lda #0
	rts
true:
	lda #1
	rts

__ccnelz:
__ccneulz:
	stz @hireg
__ccnel:
__ccneul:
	jsr longcmp
	beq false
	lda #1
	rts

__ccgtlz:
	stz @hireg
__ccgtl:
	jsr longcmp
	bvs false
	lda #1
	rts

__ccgteqlz:
	stz @hireg
__ccgteql:
	jsr longcmp
	beq true
	bvs false
	lda #1
	rts

__cclteqlz:
	stz @hireg
__cclteql:
	jsr longcmp
	bvs true
	lda #0
	rts

__ccltlz:
	stz @hireg
__ccltl:
	jsr longcmp
	beq false
	bvs true
	lda #0
	rts

__ccgtulz:
	stz @hireg
__ccgtul:
	jsr longcmp
	bcs false
	lda #1
	rts

__ccgtequlz:
	stz @hireg
__ccgtequl:
	jsr longcmp
	beq true
	bcs false
	lda #1
	rts

__ccltequlz:
	stz @hireg
__ccltequl:
	jsr longcmp
	bcs true
	lda #0
	rts

__ccltulz:
	stz @hireg
__ccltul:
	jsr longcmp
	beq false
	bcs true
	lda #0
	rts
