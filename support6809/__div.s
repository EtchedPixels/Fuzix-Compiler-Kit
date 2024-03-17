;
;	D = TOS / D signed
;
;	The rules for signed divide are
;	Dividend and remainder have the same sign
;	Quotient is negative if signs disagree
;
;	So we do the maths in unsigned then fix up
;
	.export __div
	.export __rem

;
;	The sign of the remainder of a division is not defined until C99.
;	C99 says it's the sign of the dividend.
;
__rem:
	bsr absd
	std ,s++
	ldb 4,s
	bmi negmod
	ldx 4,s			; get the dividend (unsigned)
	ldd ,--s
	jsr div16x16		; do the unsigned divide
pop2:				; X = quotient, D = remainder
	ldx ,s
	leas 4,s
	jmp ,x
negmod:
	bsr negd
	std 4,s
	ldd , --s
	ldx 4,s
	jsr div16x16
	bsr negd
	bra pop2
	

;
;	D = TOS/D signed
;
;	The sign of the result is positive if the inputs have the same
;	sign, otherwise negative
;
__div:
	ldx #0			; Count number of sign changes
	bsr absd
	std ,--s
	ldd 4,s
	bsr absd
	std 4,s
	ldd ,s++
	stx ,--s		; Save sign change
	ldx 4,s
	jsr div16x16		; do the maths
				; X = quotient, D = remainder
	ldd ,s++		; Get sign changes back in D
	rora
	tfr x,d
	bcc pop2		; low bit set -> negate
	bsr negd
	bra pop2

absd:
	bita #$80
	beq ispos
	leax 1,x		; count sign changes in X
negd:
	subd @one		; negate d
	coma
	comb
ispos:
	rts
