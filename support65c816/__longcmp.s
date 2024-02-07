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
;	Long compares
;
;
;	unsigned long - returns EQ if a match CC if hireg:A < value
;
ulongcmp:
	; Compare hireg:A with 0-3,y
	tax
	lda @hireg
	cmp 2,y
	beq uchklo
	rts
uchklo:
	txa
	cmp 0,y
	rts

longcmp:
	; Compare hireg:A with 0-3,y
	tax
	lda @hireg
	sec
	sbc 2,y
	beq chklo
setvn:
	; Now do the EOR mangling magic
	bvs vtog
	eor #0x8000
vtog:	asl a
	; C flag now the same as for unsigned compares
	rts
chklo:
	txa
	sec
	sbc 0,y
	bne setvn 
	; EQ set - same
	rts


__cceqlz:
__ccequlz:
	stz @hireg
__cceql:
__ccequl:
	jsr longcmp
	beq true
false:
	iny
	iny
	iny
	iny
	lda #0
	rts

__ccnelz:
__ccneulz:
	stz @hireg
__ccnel:
__ccneul:
	jsr longcmp
	beq false
true:
	iny
	iny
	iny
	iny
	lda #1
	rts

__ccgtlz:
	stz @hireg
__ccgtl:
	jsr longcmp
	bcc false
	bra true

__ccgteqlz:
	stz @hireg
__ccgteql:
	jsr longcmp
	beq true
	bcc false
	bra true

__cclteqlz:
	stz @hireg
__cclteql:
	jsr longcmp
	bcs true
	bra false

__ccltlz:
	stz @hireg
__ccltl:
	jsr longcmp
	beq false
	bcs true
	bra false

__ccgtulz:
	stz @hireg
__ccgtul:
	jsr ulongcmp
	bcs false
	bra true

__ccgtequlz:
	stz @hireg
__ccgtequl:
	jsr ulongcmp
	beq true
	bcs false
	bra true

__ccltequlz:
	stz @hireg
__ccltequl:
	jsr ulongcmp
	bcs true
	bra false

__ccltulz:
	stz @hireg
__ccltul:
	jsr ulongcmp
	beq false
	bcs true
	bra false
