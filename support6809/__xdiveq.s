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
	ldx ,x			; Get value
	bsr absd
	exg d,x
	bita #0x80
	bne negmod
	jsr div16x16		; do the unsigned divide
store:
	ldx ,s++
	std ,x
	rts
negmod:
	bsr negd
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
	stx, --s
	ldy #0			; Count number of sign changes
	ldx ,x
	bsr absd
	exg d,x
	bsr absd
	jsr div16x16		; do the maths
				; X = quotient, D = remainder
	ldd ,s++		; Get sign changes back in D
	rora
	tfr x,d
	bcc store		; low bit set -> negate
	bsr negd
	bra store
	
absd:
	bita #$80
	beq ispos
	leay 1,y		; count sign changes in Y
negd:
	subd @one		; negate d
	coma
	comb
ispos:
	rts
