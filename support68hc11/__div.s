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
	des
	tsy
	bsr absd
	xgdx			; save in 
	ldd 3,y
	bmi negmod
	xgdx
	jsr div16x16		; do the unsigned divide
pop2s:
	ins
pop2:				; X = quotient, D = remainder
	puly
	pulx
	jmp ,y
negmod:
	bsr negd
	xgdx
	jsr div16x16
	bsr negd
	bra pop2s
	

;
;	D = TOS/D signed
;
;	The sign of the result is positive if the inputs have the same
;	sign, otherwise negative
;
__div:
	des			; make space for counter
	tsy
	clr ,y
	bsr absd
	xgdx			; Save divisor in X
	ldd 3,y			; Get the argument
	bsr absd
	xgdx
	jsr div16x16		; do the maths
				; X = quotient, D = remainder
	; result is in X
	pulb			; get back the count info
	cmpb #1			; if we inverted one invert the result
	xgdx			; result into D
	bne pop2	
	bsr negd
	bra pop2

absd:
	bita #$80
	beq ispos
	inc ,y
negd:
	subd @one		; negate d
	coma
	comb
ispos:
	rts

