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

__ccnel:
__ccneul:
	jsr longcmp
	beq false
	lda #1
	rts

__ccgtl:
	jsr longcmp
	bvs false
	lda #1
	rts

__ccgteql:
	jsr longcmp
	beq true
	bvs false
	lda #1
	rts

__cclteql:
	jsr longcmp
	bvs true
	lda #0
	rts

__ccltl:
	jsr longcmp
	beq false
	bvs true
	lda #0
	rts

__ccgtul:
	jsr longcmp
	bcs false
	lda #1
	rts

__ccgtequl:
	jsr longcmp
	beq true
	bcs false
	lda #1
	rts

__ccltequl:
	jsr longcmp
	bcs true
	lda #0
	rts

__ccltul:
	jsr longcmp
	beq false
	bcs true
	lda #0
	rts
