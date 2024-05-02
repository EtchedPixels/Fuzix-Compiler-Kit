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

	.setcpu 6803
;
;	The sign of the remainder of a division is not defined until C99.
;	C99 says it's the sign of the dividend.
;
__xremeq:
	pshx
	bsr absd
	std @tmp
	ldd ,x
	bita #0x80
	bne negmod
	std @tmp1
	ldd @tmp
	ldx @tmp1
	jsr div16x16		; do the unsigned divide
store:
	pulx
	std ,x
	rts
negmod:
	bsr negd
	std @tmp1
	ldd @tmp
	ldx @tmp1
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
	clr @tmp4
	bsr absd
	std @tmp
	ldd ,x			; Data value
	bsr absd
	std @tmp1
	ldd @tmp
	ldx @tmp1
	jsr div16x16		; do the maths
				; X = quotient, D = remainder
	stx @tmp
	ldd @tmp
	ror @tmp4
	bcs store		; low bit set -> negate
	bsr negd
	bra store
	
absd:
	bita #$80
	beq ispos
	inc @tmp4		; count sign changes in Y
negd:
	subd @one		; negate d
	coma
	comb
ispos:
	rts
