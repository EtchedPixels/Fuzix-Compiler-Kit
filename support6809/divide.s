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
;	Assumed to preserve Y in the eq helpers (but easily changed if
;	useful)
;
	.export div16x16

	.code

div16x16:
	; TODO - should be we spot div by 0 and trap out ?
	std ,--s
	ldb #16			; bit count
	stb ,-s			; counter
	clra
	clrb
loop:
	exg d,x
	lslb			; shift X left one bit at a time (dividend)
	rola
	exg d,x
	rolb
	rola
	leax 1,x
	subd 1,s		; divisor
	bcc skip
	addd 1,s
	leax -1,x
skip:
	dec ,s
	bne loop
	leas 3,s
	rts
