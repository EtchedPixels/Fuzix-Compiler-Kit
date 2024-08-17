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
	staa @tmp1		; divisor
	stab @tmp1+1
	clra
	clrb
	stx @tmp
	ldx #16			; bit count
loop:
	asl @tmp+1			; shift X left one bit at a time (dividend)
	rol @tmp
	rolb
	rola
	inc @tmp+1		; low bit is currently clear
	subb @tmp1+1		; divisor
	sbca @tmp1
	bcc skip
	addb @tmp1+1
	adca @tmp1
	dec @tmp+1		; set low bit back clear
skip:
	dex
	bne loop
	ldx @tmp
	rts
