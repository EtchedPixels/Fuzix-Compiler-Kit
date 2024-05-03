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

	.setcpu 6803
;
;	The sign of the remainder of a division is not defined until C99.
;	C99 says it's the sign of the dividend.
;
__rem:
	tsx
	bsr absd
	pshb
	psha
	ldd 2,x
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
	std 2,x
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
	ldd 2,x
	bsr absd
	std 2,x
	pula
	pulb
	ldx 2,x
	jsr div16x16		; do the maths
				; X = quotient, D = remainder
	stx @tmp		; save quotient for fixup
	ldaa @tmp4
	rora
	bcc divdone		; low bit set -> negate
	ldd @tmp
	bsr negd
	jmp __pop2
divdone:
	ldd @tmp
	jmp __pop2

absd:
	bita #$80
	beq ispos
	inc @tmp4
negd:
	subd @one		; negate d
	coma
	comb
ispos:
	rts
