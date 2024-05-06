;
;	Do a 32bit divide unsigned
;
	.setcpu	4
	.export __divl
	.export __reml
	.code

__divl:
	stb	(-s)
	ldb	(__hireg)
	stb	(-s)
	lda	1
	jsr	signfix	; X is sign info and saved by the jsr
	jsr	div32x32
	; Result is in the TOS value
	lda	6(s)
	ldb	8(s)
dosign:
	ori	x,x
	bz	noneg
	ivr	a
	ivr	b	; negate result
	inr	b
	bnz	norip
	inr	a
norip:
noneg:
	sta	(__hireg)
	lda	8	; clean up frame
	add	a,s
	rsr

__reml:
	stb	(-s)
	ldb	(__hireg)
	stb	(-s)
	cla
	jsr	signfix2
	jsr	div32x32
	bra	dosign

; Fix signs for divide, turn both positive and count signs
signfix:
	clr	x
	lda	2(s)	; value just pushed
	bp	nosh1
	dcx
; Comm part of the fixup. For rem we don't care about the sign of the
; second value for sign fixup
sfcommon:
	ldb	4(s)
	ivr	b
	iva
	inr	b
	bnz	norip2
	ina
norip2:	sta	2(s)
	stb	4(s)
nosh1:
	lda	8(s)
	bp	nosh2
	ldb	10(s)
	ivr	b
	iva
	inr	b
	bnz	norip3
	ina
norip3:	sta	8(s)
	stb	10(s)
	inx
nosh2:
	rsr
; Version for remainder
signfix2:
	clr	x
	lda	2(s)
	bp	nosh1
	bra	sfcommon
