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
	tsx
	bsr absd
	pshb
	psha
	ldaa 2,x
	bita #$80		; sign bit of dividend
	bne negmod
	ldx 2,x			; get the dividend (unsigned)
	pula
	pulb
	jsr div16x16		; do the unsigned divide
				; X = quotient, D = remainder
	jmp __pop2
negmod:
	bsr negd
	staa 2,x
	stab 3,x
	pula
	pulb
	ldx 2,x
	jsr div16x16
	bsr negd
	jmp __pop2
	

;
;	D = TOS/D signed
;
;	The sign of the result is positive if the inputs have the same
;	sign, otherwise negative
;
__div:
	clr @tmp4
	tsx
	bsr absd
	pshb
	psha
	ldaa 2,x
	ldab 3,x
	bsr absd
	staa 2,x
	stab 3,x
	pula
	pulb
	ldx 2,x
	jsr div16x16		; do the maths
				; X = quotient, D = remainder
	stx @tmp		; save quotient for fixup
	ldaa @tmp4
	rora
	bcc divdone		; low bit set -> negate
	ldaa @tmp
	ldab @tmp+1
	bsr negd
	jmp __pop2
divdone:
	ldaa @tmp
	ldab @tmp+1
	jmp __pop2

absd:
	bita #$80
	beq ispos
	inc @tmp4
negd:
	subb #1			; negate d
	sbca #0
	coma
	comb
ispos:
	rts
