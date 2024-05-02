;
;	This algorithm is taken from the manual
;
;	This is the classic division algorithm
;
;	On entry D holds the divisor and X holds the dividend
;
;	work = 0
;	loop for each bit in size (tracked in tmp)
;		shift dividend left (X)
;		rotate left into work (D)
;		set low bit of dividend (X)
;		subtract divisor (@tmp1) from work
;		if work < 0
;			add divisor (@tmp1) back to work
;			clear lsb of X
;		end
;	end loop
;
;	On exit X holds the quotient, D holds the remainder
;
;
	.export div16x16

	.code

div16x16:
	; TODO - should be we spot div by 0 and trap out ?
	pshb
	psha
	ldab #16		; bit count
	pshb			; counter
	clra
	clrb
	tsy
loop:
	xgdx
	lslb			; shift X left one bit at a time (dividend)
	rola
	xgdx
	rolb
	rola
	inx
	subd 1,y		; divisor
	bcc skip
	addd 1,y
	dex
skip:
	dec ,y
	bne loop
	ins
	ins
	ins
	rts
