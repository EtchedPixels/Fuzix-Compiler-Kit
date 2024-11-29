;
;	D = ,X / D signed
;
;	The rules for signed divide are
;	Dividend and remainder have the same sign
;	Quotient is negative if signs disagree
;
;	So we do the maths in unsigned then fix up
;
	.export __xdiveq
	.export __xremeq

;
;	The sign of the remainder of a division is not defined until C99.
;	C99 says it's the sign of the dividend.
;
__xremeq:
	stx ,--s		; save pointer
	ldy #0
	bsr absd
	ldx ,x
	exg d,x
	bita #0x80
	bne negmod
	exg d,x
	lbsr div16x16		; do the unsigned divide
store:
	ldx ,s++
	std ,x
	rts
negmod:
	bsr negd
	exg d,x
	lbsr div16x16
	bsr negd
	bra store
	

;
;	D = TOS/D signed
;
;	The sign of the result is positive if the inputs have the same
;	sign, otherwise negative
;
__xdiveq:
	stx, --s
	ldy #0			; Count number of sign changes
	ldx ,x			; Data value
	bsr absd
	exg d,x
	bsr absd
	exg d,x
	lbsr div16x16		; do the maths
				; X = quotient, D = remainder
	tfr x,d
	cmpy #1
	bne store		; low bit set -> negate
	bsr negd
	bra store
	
absd:
	bita #$80
	beq ispos
	leay 1,y		; count sign changes in Y
negd:
	nega
	negb
	sbca #0			; negate d
ispos:
	rts
