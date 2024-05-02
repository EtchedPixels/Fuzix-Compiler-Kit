;
;	D = ,X / D signed
;
;	The rules for signed divide are
;	Dividend and remainder have the same sign
;	Quotient is negative if signs disagree
;
;	So we do the maths in unsigned then fix up
;
	.export __xdivequc
	.export __xremequc

	.setcpu 6803

__xremequc:
	pshx			; save pointer
	clra
	std @tmp
	ldd ,x
	clra
	std @tmp2
	ldd @tmp
	ldx @tmp2
	jsr div16x16		; do the unsigned divide
store:
	pulx
	stab ,x
	rts
	
__xdivequc:
	pshx
	clra
	std @tmp
	ldd ,x			; Data value
	clra
	std @tmp2
	ldd @tmp
	ldx @tmp2
	jsr div16x16		; do the maths
				; X = quotient, D = remainder
	stx @tmp
	ldd @tmp
	bra store
