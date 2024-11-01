;
;	Adds the value of the accumulator to X.
;	messing up D
;
	.export __abx
	.export __adx
	.code

__abx:
	stx @tmp	; X where we can manipulate it
	addb @tmp+1
	adca #0
	stab @tmp+1
	staa @tmp
	ldx @tmp
	rts

__adx:
	stx @tmp	; X where we can manipulate it
	addb @tmp+1
	adca @tmp
	stab @tmp+1
	staa @tmp
	ldx @tmp
	rts
