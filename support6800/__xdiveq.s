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
	stx @tmp5
	bsr absd
	staa @tmp
	stab @tmp+1
	ldaa ,x
	ldab 1,x
	bita #0x80
	bne negmod
	staa @tmp1
	stab @tmp1+1
	ldaa @tmp
	ldab @tmp+1
	ldx @tmp1
	jsr div16x16		; do the unsigned divide
store:
	ldx @tmp5
	staa ,x
	stab 1,x
	rts
negmod:
	bsr negd
	staa @tmp1
	stab @tmp1+1
	ldaa @tmp
	ldab @tmp+1
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
	stx @tmp5
	clr @tmp4
	bsr absd
	staa @tmp
	stab @tmp+1
	ldaa ,x			; Data value
	ldab 1,x
	bsr absd
	staa @tmp1
	stab @tmp+1
	ldaa @tmp
	ldab @tmp+1
	ldx @tmp1
	jsr div16x16		; do the maths
				; X = quotient, D = remainder
	stx @tmp
	ldaa @tmp
	ldab @tmp+1
	ror @tmp4
	bcs store		; low bit set -> negate
	bsr negd
	bra store
	
absd:
	bita #$80
	beq ispos
	inc @tmp4		; count sign changes in Y
negd:
	subb #1			; negate d
	sbca #0
	coma
	comb
ispos:
	rts
