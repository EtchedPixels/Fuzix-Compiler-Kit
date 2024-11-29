;
;	D = ,X / D signed
;
;	The rules for signed divide are
;	Dividend and remainder have the same sign
;	Quotient is negative if signs disagree
;
;	So we do the maths in unsigned then fix up
;
	.export __xdiveqc
	.export __xremeqc

	.setcpu 6303
;
;	The sign of the remainder of a division is not defined until C99.
;	C99 says it's the sign of the dividend.
;
__xremeqc:
	pshx			; save pointer
	bsr sex
	bsr absd
	ldx ,x
	xgdx
	tab
	bsr sex
	bita #0x80
	bne negmod
	xgdx
	jsr div16x16		; do the unsigned divide
store:
	pulx
	stab ,x
	rts
negmod:
	bsr negd
	xgdx
	jsr div16x16
	negb
	bra store
	

;
;	D = TOS/D signed
;
;	The sign of the result is positive if the inputs have the same
;	sign, otherwise negative
;
__xdiveqc:
	pshx
	clr @tmp2		; Count number of sign changes
	ldx ,x			; Data value
	bsr sex
	bsr absd
	xgdx
	tab
	bsr sex
	bsr absd
	xgdx
	jsr div16x16		; do the maths
				; X = quotient, D = remainder
	xgdx
	ror @tmp2
	bcc store		; low bit set -> negate
	negb
	bra store
	
absd:
	bita #$80
	beq ispos
	inc @tmp2		; count sign changes in tmp2
negd:
	nega
	negb
	sbca #0
ispos:
	rts

sex:	clra
	bitb #0x80
	beq nomin
	deca
nomin:	rts

