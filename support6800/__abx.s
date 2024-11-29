;
;	Adds the value of the accumulator to X.
;	messing up D
;
	.export __abx
	.export __adx
	.code

__abx:
	clra
__adx:
	stx @tmp	; X where we can manipulate it
	addb @tmp+1
	adca @tmp
	stab @tmp+1
	staa @tmp
	ldx @tmp
	rts
