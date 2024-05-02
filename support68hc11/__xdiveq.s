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
	pshx
	ldy #0
	bsr absd
	ldx ,x
	xgdx
	bita #0x80
	bne negmod
	xgdx
	jsr div16x16		; do the unsigned divide
store:
	pulx
	std ,x
	rts
negmod:
	bsr negd
	xgdx
	jsr div16x16
	bsr negd
	bra store
	

;
;	D = TOS/D signed
;
;	The sign of the result is positive if the inputs have the same
;	sign, otherwise negative
;
__xdiveq:
	pshx
	ldy #0			; Count number of sign changes
	ldx ,x			; Data value
	bsr absd
	xgdx
	bsr absd
	xgdx
	pshy
	jsr div16x16		; do the maths
				; X = quotient, D = remainder
	puly
	xgdx
	cpy #1
	bne store		; low bit set -> negate
	bsr negd
	bra store
	
absd:
	bita #$80
	beq ispos
	iny			; count sign changes in Y
negd:
	subd @one		; negate d
	coma
	comb
ispos:
	rts
