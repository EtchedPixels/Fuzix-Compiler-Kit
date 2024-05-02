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

__xremequc:
	pshx			; save pointer
	clra
	ldx ,x
	xgdx
	clra
	xgdx
	jsr div16x16		; do the unsigned divide
store:
	pulx
	stab ,x
	rts
	
__xdivequc:
	pshx
	ldx ,x			; Data value
	clra
	xgdx
	clra
	xgdx
	jsr div16x16		; do the maths
				; X = quotient, D = remainder
	xgdx
	bra store
