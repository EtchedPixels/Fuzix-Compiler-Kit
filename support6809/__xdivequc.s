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
	stx ,--s		; save pointer
	clra
	ldx ,x
	exg d,x
	clra
	exg d,x
	jsr div16x16		; do the unsigned divide
store:
	ldx ,s++
	stb ,x
	rts
	
__xdivequc:
	stx, --s
	ldx ,x			; Data value
	clra
	exg d,x
	clra
	exg d,x
	jsr div16x16		; do the maths
				; X = quotient, D = remainder
	tfr x,d
	bra store
