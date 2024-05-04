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
	.export __regdiv
	.export __regmod

;
;	The sign of the remainder of a division is not defined until C99.
;	C99 says it's the sign of the dividend.
;
__rem:
	bsr absd
	tfr d,x			; into X
	ldd 2,s
	bmi negmod
	exg d,x
	lbsr div16x16		; do the unsigned divide
pop2:				; X = quotient, D = remainder
	ldx ,s
	leas 4,s
	jmp ,x
negmod:
	bsr negd
	exg d,x
	lbsr div16x16
	bsr negd
	bra pop2
	

;
;	D = TOS/D signed
;
;	The sign of the result is positive if the inputs have the same
;	sign, otherwise negative
;
__div:
	ldy #0			; Count number of sign changes
	bsr absd
	tfr d,x			; Save divisor in X
	ldd 2,s			; Get the argument
	bsr absd
	exg d,x
	lbsr div16x16		; do the maths
				; X = quotient, D = remainder
	tfr x,d
	cmpy #1			; if we inverted one invert the result
	bne pop2	
	bsr negd
	bra pop2

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

;
;	Register versions
;

__regdiv:
	pshs u
	lbsr __div
	tfr d,u
	rts

__regmod:
	pshs u
	lbsr __rem
	tfr d,u
	rts
